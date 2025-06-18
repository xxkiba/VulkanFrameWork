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
            uint32_t imageCount,
            VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT,
            VkFormat depthFormat = VK_FORMAT_D32_SFLOAT) {
			return std::make_shared<OffscreenRenderTarget>(device, commandPool, width, height, imageCount, colorFormat, depthFormat);
        }

        OffscreenRenderTarget(const Wrapper::Device::Ptr& device,
            const Wrapper::CommandPool::Ptr& commandPool,
            uint32_t width, uint32_t height,
			uint32_t imageCount,
            VkFormat colorFormat,
            VkFormat depthFormat);

        ~OffscreenRenderTarget();
    public:

        void setClearColor(int idx, float r, float g, float b, float a);
        void setClearDepth(float depth, uint32_t stencil);
        void recreate(uint32_t width, uint32_t height); // resize/÷ÿΩ®

		Wrapper::Image::Ptr getRenderTargetImage(int idx) const { return mRenderTargetImages[idx]; }
        const std::vector<Wrapper::Image::Ptr>& getRenderTargetImages() const { return mRenderTargetImages; }


		std::vector<VkFramebuffer> getOffScreenFramebuffers() const { return mOffScreenFramebuffers; }
        Wrapper::RenderPass::Ptr getRenderPass() const { return mRenderPass; }

        uint32_t getWidth() const { return mWidth; }
        uint32_t getHeight() const { return mHeight; }


        VkCommandBuffer beginRendering(VkCommandBuffer cmd = nullptr);


    private:
		void createImageEntities();
        void createRenderPass();
        void createFramebuffer();
        void cleanup();

    private:
        Wrapper::Device::Ptr mDevice;
        Wrapper::CommandPool::Ptr mCommandPool;
        uint32_t mWidth, mHeight;
		uint32_t mImageCount; // Number of images in the offscreen render target,by default the same as the swap chain image count
        int mColorBufferCount;
        VkFormat mColorFormat, mDepthFormat;

        // Render Target Images
        std::vector<Wrapper::Image::Ptr> mRenderTargetImages{}; // offscreen render target

        // Depth image for the swapchain
        std::vector<Wrapper::Image::Ptr> mDepthImages{};

        // Multisampling image for the swapchain, transient image
        std::vector<Wrapper::Image::Ptr> mMultisampleImages{};


        Wrapper::Image::Ptr mDepthAttachment;
        std::vector<VkClearValue> mClearValues;
        int mDepthBufferIndex;

        Wrapper::RenderPass::Ptr mRenderPass{ VK_NULL_HANDLE };
		std::vector<VkFramebuffer> mOffScreenFramebuffers{};

	};
}