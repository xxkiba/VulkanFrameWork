#include "Material.h"

namespace FF {
    Material::Material() {}
    
    void Material::init(const Wrapper::Device::Ptr& device,
        const Wrapper::CommandPool::Ptr& commandPool,
        std::vector<std::string> texturePaths,
        int frameCount) {

        // 1. Create texture
		mTextures.resize(texturePaths.size());
        for (size_t i = 0; i < texturePaths.size(); i++) {
            auto textureParam = Wrapper::UniformParameter::create();
            mTextures[i] = Texture::create(device, commandPool, texturePaths[i]);
        }

        std::vector<Wrapper::UniformParameter::Ptr> params;
        

        // 2. Create uniform parameter for the texture
        auto textureParam = Wrapper::UniformParameter::create();
        textureParam->mBinding = 0;
        textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureParam->mCount = static_cast<uint32_t>(mTextures.size()); // Number of textures
        textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		textureParam->mTextures = mTextures; // vector of textures
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

