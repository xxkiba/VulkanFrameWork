#include "Material.h"

namespace FF {
    Material::Material() {}
    
    void Material::init(const Wrapper::Device::Ptr& device,
        const Wrapper::CommandPool::Ptr& commandPool,
        const std::string& texturePath,
        int frameCount) {
        // 1. Create texture
        mTexture = Texture::create(device, commandPool, texturePath);

        // 2. Create uniform parameter for the texture
        auto textureParam = Wrapper::UniformParameter::create();
        textureParam->mBinding = 0; // Use 0 if this is the only resource in this set
        textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureParam->mCount = 1;
        textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        textureParam->mTexture = mTexture;

        std::vector<Wrapper::UniformParameter::Ptr> params = { textureParam };

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

