#include "texture.h"


#define STB_IMAGE_IMPLEMENTATION

#include "../stb_image.h"

namespace FF {
	Texture::Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& filePath)
		: mDevice(device),mCommandPool(commandPool), mFilePath(filePath) {

		int texWidth, texHeight, texSize, texChannels;
		//stbi_set_flip_vertically_on_load(true); // Flip the image vertically to match Vulkan's coordinate system
		stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels || texWidth == 0 || texHeight == 0) {
			throw std::runtime_error("Error: failed to load image or invalid dimensions! Path: " + filePath);
		}

		if (!pixels) {
			throw std::runtime_error("Error: failed to read image data!");
		}

		texSize = texWidth * texHeight * 4;

		mImage = Wrapper::Image::create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT);

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		mImage->setImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			subresourceRange,
			mCommandPool);

		mImage->fillImageData(texSize, (void*)pixels, mCommandPool);

		mImage->setImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			subresourceRange,
			mCommandPool);


		// Free the image data!!
		stbi_image_free(pixels);

		mSampler = Wrapper::Sampler::create(mDevice);

		mImageInfo.imageLayout = mImage->getLayout();
		mImageInfo.imageView = mImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();

	}

	// Cubemap constructor
    Texture::Texture(const Wrapper::Device::Ptr& device,
        const Wrapper::CommandPool::Ptr& commandPool,
        const std::array<std::string, 6>& cubemapPaths)
        : mDevice(device), mCommandPool(commandPool) {

        int texWidth = 0, texHeight = 0, texChannels = 0;
        std::vector<stbi_uc*> facePixels(6);
        size_t faceSize = 0;
		//stbi_set_flip_vertically_on_load(true); // Flip the image vertically to match Vulkan's coordinate system
		// load each face of the cubemap
        for (int i = 0; i < 6; ++i) {
            int w, h, c;
            facePixels[i] = stbi_load(cubemapPaths[i].c_str(), &w, &h, &c, STBI_rgb_alpha);

            if (!facePixels[i] || w == 0 || h == 0) {
				// clean up previously loaded faces
                for (int j = 0; j < i; ++j) {
                    if (facePixels[j]) stbi_image_free(facePixels[j]);
                }
                throw std::runtime_error("Error: failed to load cubemap face: " + cubemapPaths[i]);
            }

			// make sure all faces have the same dimensions
            if (i == 0) {
                texWidth = w;
                texHeight = h;
                texChannels = c;
                faceSize = w * h * 4; // RGBA
            }
            else {
                if (w != texWidth || h != texHeight) {
					// clean up previously loaded faces
                    for (int j = 0; j <= i; ++j) {
                        if (facePixels[j]) stbi_image_free(facePixels[j]);
                    }
                    throw std::runtime_error("Error: cubemap faces must have same dimensions!");
                }
            }
        }

		// create consecutive memory for all faces
        size_t totalSize = faceSize * 6;
        stbi_uc* allPixels = new stbi_uc[totalSize];

		// Copy each face's pixels into the consecutive memory
        for (int i = 0; i < 6; ++i) {
            memcpy(allPixels + i * faceSize, facePixels[i], faceSize);
			stbi_image_free(facePixels[i]); // Free each face's pixels after copying
        }

		// Create the cubemap image
        mImage = Wrapper::Image::create(
            mDevice, texWidth, texHeight,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,true);

		// Set the image layout for transfer
        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 6; // Cubemap has 6 faces

        mImage->setImageLayout(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            subresourceRange,
            mCommandPool);

		// Fill the image with pixel data
        mImage->fillImageData(totalSize, (void*)allPixels, mCommandPool,true);

        mImage->setImageLayout(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            subresourceRange,
            mCommandPool);

		// Free the allPixels memory
        delete[] allPixels;

		// Create the sampler for the cubemap
        mSampler = Wrapper::Sampler::create(mDevice);
        mImageInfo.imageLayout = mImage->getLayout();
        mImageInfo.imageView = mImage->getImageView();
        mImageInfo.sampler = mSampler->getSampler();
    }

	Texture::~Texture() {
		if (mSampler != nullptr) {
			mSampler.reset();
		}
		if (mImage != nullptr) {
			mImage.reset();
		}
		mDevice = nullptr;
		mCommandPool = nullptr;
	}
}