#pragma once
#include "texture/texture.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/commandPool.h"

namespace FF {
    class Material {
    public:
        using Ptr = std::shared_ptr<Material>;
        static Ptr create() {
            return std::make_shared<Material>();
        }

        Material();
        ~Material();

        void init(const Wrapper::Device::Ptr& device,
            const Wrapper::CommandPool::Ptr& commandPool,
            std::vector<std::string> texturePaths,
            int frameCount);

        [[nodiscard]] auto getDescriptorLayout() const {
            return mDescriptorLayout;
        }
        [[nodiscard]] auto getDescriptorPool() const {
            return mDescriptorPool;
        }
        [[nodiscard]] auto getDescriptorSet(int frameCount) const {
            return mDescriptorSet->getDescriptorSet(frameCount);
        }

    private:
        std::vector<Texture::Ptr> mTextures{ nullptr };
        Wrapper::DescriptorSetLayout::Ptr mDescriptorLayout{ nullptr };
        Wrapper::DescriptorPool::Ptr mDescriptorPool{ nullptr };
        Wrapper::DescriptorSet::Ptr mDescriptorSet{ nullptr };
    };
}

