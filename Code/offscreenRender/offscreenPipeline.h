#pragma once
#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/pipeline.h"
#include "../vulkanWrapper/renderPass.h"
#include "../vulkanWrapper/shader.h"


namespace FF {
    class OffscreenPipeline {
    public:
        using Ptr = std::shared_ptr<OffscreenPipeline>;
        static Ptr create(const Wrapper::Device::Ptr& device) {
			return std::make_shared<OffscreenPipeline>(device);
        }

        OffscreenPipeline(const Wrapper::Device::Ptr& device);
        ~OffscreenPipeline();

        void build(
            const Wrapper::RenderPass::Ptr& renderPass,
            uint32_t width, uint32_t height,
            const std::string& vertexShaderFile, const std::string& fragShaderFile,
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkVertexInputBindingDescription>& bindingDes,
            const std::vector<VkVertexInputAttributeDescription>& attributeDes,
            const std::vector<VkPushConstantRange>* pushConstantRanges = nullptr,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
			VkFrontFace inFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			bool needFlipVewport = true,
			bool enableDynamicViewPort = false
        );

        void buildScreenQuadPipeline(const Wrapper::RenderPass::Ptr& renderPass,
            uint32_t width, uint32_t height,
            const std::string& vertexShaderFile, const std::string& fragShaderFile,
            const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
            const std::vector<VkPushConstantRange>* pushConstantRanges = nullptr,
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
            VkFrontFace inFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            bool needFlipVewport = true,
            bool enableDynamicViewPort = false);

        Wrapper::Pipeline::Ptr getPipeline() const { return mPipeline; }

    private:
        Wrapper::Device::Ptr mDevice;
        Wrapper::Pipeline::Ptr mPipeline;
        uint32_t mWidth, mHeight;
    };
}
