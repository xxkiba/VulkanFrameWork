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