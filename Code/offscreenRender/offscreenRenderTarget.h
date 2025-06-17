#pragma once

#include "../base.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/renderPass.h"
#include "../vulkanWrapper/image.h"
#include "../vulkanWrapper/commandBuffer.h"
#include "../vulkanWrapper/commandPool.h"

namespace FF {
	class OffscreenRenderTarget {
    public:
        using Ptr = std::shared_ptr<OffscreenRenderTarget>;
        static Ptr create(const Wrapper::Device::Ptr& device,
            const Wrapper::CommandPool::Ptr& commandPool,
            uint32_t width, uint32_t height,
            int colorCount,
            VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM,
            VkFormat depthFormat = VK_FORMAT_D32_SFLOAT);

        OffscreenRenderTarget(const Wrapper::Device::Ptr& device,
            const Wrapper::CommandPool::Ptr& commandPool,
            uint32_t width, uint32_t height,
            int colorCount,
            VkFormat colorFormat,
            VkFormat depthFormat);

        ~OffscreenRenderTarget();

        void setClearColor(int idx, float r, float g, float b, float a);
        void setClearDepth(float depth, uint32_t stencil);
        void recreate(uint32_t width, uint32_t height); // resize/ÖØ½¨

        VkFramebuffer getFramebuffer() const { return mFramebuffer; }
        Wrapper::RenderPass::Ptr getRenderPass() const { return mRenderPass; }
        VkImageView getColorImageView(int idx) const { return mColorAttachments.at(idx)->getImageView(); }
        VkImageView getDepthImageView() const { return mDepthAttachment->getImageView(); }
        uint32_t getWidth() const { return mWidth; }
        uint32_t getHeight() const { return mHeight; }
        int getColorBufferCount() const { return mColorAttachments.size(); }

        VkCommandBuffer beginRendering(VkCommandBuffer cmd = nullptr);

    private:
        void createAttachments();
        void createRenderPass();
        void createFramebuffer();
        void cleanup();

    private:
        Wrapper::Device::Ptr mDevice;
        Wrapper::CommandPool::Ptr mCommandPool;
        uint32_t mWidth, mHeight;
        int mColorBufferCount;
        VkFormat mColorFormat, mDepthFormat;

        std::vector<Wrapper::Image::Ptr> mColorAttachments;
        Wrapper::Image::Ptr mDepthAttachment;
        std::vector<VkClearValue> mClearValues;
        int mDepthBufferIndex;

        Wrapper::RenderPass::Ptr mRenderPass{ VK_NULL_HANDLE };
        VkFramebuffer mFramebuffer{ VK_NULL_HANDLE };
	};
}