#include "Material.h"

namespace FF {
    Material::Material() {
		mTexturePaths.clear();
		mAttachedImages.clear();
		mTextures.clear();
    }
    

    void Material::attachTexturePath(const std::string& path) {
        mTexturePaths.push_back(path);
    }

    void Material::attachTexturePaths(const std::vector<std::string>& paths) {
        mTexturePaths.insert(mTexturePaths.end(), paths.begin(), paths.end());
    }


	void Material::attachImages(const std::vector<Wrapper::Image::Ptr>& perFrameImages) {
		mAttachedImagesPerFrame.push_back(perFrameImages);
	}
    void Material::init(const Wrapper::Device::Ptr& device,
        const Wrapper::CommandPool::Ptr& commandPool,
        int frameCount) {


        std::vector<Wrapper::UniformParameter::Ptr> params;
        

        // 1. Create uniform parameter for the texture
        auto textureParam = Wrapper::UniformParameter::create();
        textureParam->mBinding = 0;
        textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureParam->mCount = static_cast<uint32_t>(mTexturePaths.size() + mAttachedImagesPerFrame.size()); // Number of textures
        textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// 2. Create textures for each frame
		textureParam->mTextures.resize(frameCount); // Resize to frameCount, each frame will have its own textures
        for (int i = 0; i < frameCount; i++) {
            for (const auto& path : mTexturePaths) {
                textureParam->mTextures[i].push_back(Texture::create(device, commandPool, path));
            }
            for (const auto& images : mAttachedImagesPerFrame) {
                textureParam->mTextures[i].push_back(Texture::createFromImage(device, images[i]));
            }
        }
        params.push_back(textureParam);


        // 3. Create descriptor set layout
        mDescriptorLayout = Wrapper::DescriptorSetLayout::create(device);
        mDescriptorLayout->build(params);

        // 4. Create descriptor pool
        mDescriptorPool = Wrapper::DescriptorPool::create(device);
        mDescriptorPool->build(params, frameCount);

        // 5. Create descriptor set
        mDescriptorSet = Wrapper::DescriptorSet::create(device, params, mDescriptorLayout, mDescriptorPool, frameCount);
    }

    Material::~Material() {
        // Resource release if needed (smart pointers usually handle this)
    }
}

