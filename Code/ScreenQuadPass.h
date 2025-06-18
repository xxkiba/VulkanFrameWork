#pragma once
#include "base.h"

#include "vulkanWrapper/device.h"
#include "vulkanWrapper/commandBuffer.h"
#include "vulkanWrapper/renderPass.h"
#include "vulkanWrapper/pipeline.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/descriptorSet.h"
#include "offscreenRender/OffscreenRenderTarget.h"

namespace FF {
    class ScreenQuadPass {
    public:
        using Ptr = std::shared_ptr<ScreenQuadPass>;

        // RenderTarget and RenderPass
        static Ptr create(
            const Wrapper::Device::Ptr& device,
            const OffscreenRenderTarget::Ptr& outputTarget,
            const std::string& vsPath,
            const std::string& fsPath);

        ScreenQuadPass(
            const Wrapper::Device::Ptr& device,
            const OffscreenRenderTarget::Ptr& outputTarget,
            const std::string& vsPath,
            const std::string& fsPath);
        ~ScreenQuadPass();

        // input image
        void setInputImage(VkImageView imageView, VkSampler sampler);

        // build descriptions/pipeline
        void build();

		// begin rendering
		void render(const Wrapper::CommandBuffer::Ptr& cmdBuf, const Wrapper::RenderPass::Ptr& renderPass, VkFramebuffer& frameBuffer);

        
        Wrapper::Pipeline::Ptr getPipeline() const { return mPipeline; }

    private:
        void createDescriptor();
        void createPipeline();

        Wrapper::Device::Ptr mDevice;
        OffscreenRenderTarget::Ptr mOutputTarget;

        VkImageView mInputImageView{ VK_NULL_HANDLE };
        VkSampler mInputSampler{ VK_NULL_HANDLE };

        // descriptor layout/pool/set
        Wrapper::DescriptorSetLayout::Ptr mDescriptorLayout{ nullptr };
        Wrapper::DescriptorPool::Ptr mDescriptorPool{ nullptr };
        Wrapper::DescriptorSet::Ptr mDescriptorSet{ nullptr };

		// pipeline
        Wrapper::Pipeline::Ptr mPipeline{ nullptr };

        std::string mVertexShaderPath, mFragmentShaderPath;
        bool mBuilt{ false };
    };
}
