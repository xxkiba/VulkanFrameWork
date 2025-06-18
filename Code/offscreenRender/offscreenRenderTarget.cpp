#include "offscreenRenderTarget.h"

namespace FF {
    OffscreenRenderTarget::OffscreenRenderTarget(
        const Wrapper::Device::Ptr& device,
        const Wrapper::CommandPool::Ptr& commandPool,
        uint32_t width, uint32_t height,
        uint32_t imageCount,
        VkFormat colorFormat,
        VkFormat depthFormat)
		: mDevice(device), mCommandPool(commandPool), mWidth(width), mHeight(height), mImageCount(imageCount),
         mColorFormat(colorFormat), mDepthFormat(depthFormat)
    {
        createImageEntities();
        createRenderPass();
        createFramebuffer();
    }

    OffscreenRenderTarget::~OffscreenRenderTarget() {
        cleanup();
    }

    void OffscreenRenderTarget::cleanup() {
		mOffScreenFramebuffers.clear();
		mRenderPass.reset();
        mDepthAttachment.reset();
        mClearValues.clear();
    }

    void OffscreenRenderTarget::createImageEntities() {
		mRenderTargetImages.resize(mImageCount);
		mMultisampleImages.resize(mImageCount);
		mDepthImages.resize(mImageCount);



		VkImageSubresourceRange renderTargetSubresourceRange{}; // Subresource range for the render target images
		renderTargetSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Aspect of the render target images
		renderTargetSubresourceRange.baseMipLevel = 0; // Base mip level of the render target images
		renderTargetSubresourceRange.levelCount = 1; // Number of mip levels in the render target images
		renderTargetSubresourceRange.baseArrayLayer = 0; // Base array layer of the render target images
		renderTargetSubresourceRange.layerCount = 1; // Number of array layers in the render target images

        VkImageSubresourceRange multisampleSubresourceRange{}; // Subresource range for the multisample image
        multisampleSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Aspect of the multisample image
        multisampleSubresourceRange.baseMipLevel = 0; // Base mip level of the multisample image
        multisampleSubresourceRange.levelCount = 1; // Number of mip levels in the multisample image
        multisampleSubresourceRange.baseArrayLayer = 0; // Base array layer of the multisample image
        multisampleSubresourceRange.layerCount = 1; // Number of array layers in the multisample image

        VkImageSubresourceRange depthSubresourceRange{}; // Subresource range for the depth image
        depthSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Aspect of the depth image
        depthSubresourceRange.baseMipLevel = 0; // Base mip level of the depth image
        depthSubresourceRange.levelCount = 1; // Number of mip levels in the depth image
        depthSubresourceRange.baseArrayLayer = 0; // Base array layer of the depth image
        depthSubresourceRange.layerCount = 1; // Number of array layers in the depth image


		for (uint32_t i = 0; i < mImageCount; ++i) {

			mRenderTargetImages[i] = Wrapper::Image::create(
				mDevice,
				mWidth, mHeight,
				mColorFormat,
				VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

			mRenderTargetImages[i]->setImageLayout(
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				renderTargetSubresourceRange,
				mCommandPool); // Set the image layout for the render target image

            mMultisampleImages[i] = Wrapper::Image::createRenderTargetImage(
                mDevice,
                mWidth,
                mHeight,
                mColorFormat); // Create multisample images for the swap chain

            mMultisampleImages[i]->setImageLayout(
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                multisampleSubresourceRange,
                mCommandPool); // Set the image layout for the multisample image


            mDepthImages[i] = Wrapper::Image::createDepthImage(mDevice, mWidth, mHeight);

            mDepthImages[i]->setImageLayout(
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                depthSubresourceRange,
                mCommandPool); // Set the image layout for the depth image



        }
    }


    void OffscreenRenderTarget::createRenderPass() {
        mRenderPass = Wrapper::RenderPass::create(mDevice);
		// Create a render pass for the offscreen render target
        // 
		// 0: Render target color attachment, serves as a texture for the next renderpass
        // 1: Resolve image(MultiSample)
        // 2: Depth attachment

        // 0: Final output color attachment, created by the swap chain, target of resolve operation, is also the target to be set in subpass
        // Description of input canvas
        VkAttachmentDescription finalAttachment{};
		finalAttachment.format = mColorFormat;
        finalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        finalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        finalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        finalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        finalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        finalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		finalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // The final layout of the final attachment is shader read only optimal, which is suitable for sampling in shaders
        mRenderPass->addAttachment(finalAttachment);

		// Description of indexes and format of the attachments before enter subpass
        VkAttachmentReference finalAttachmentRef{};
        finalAttachmentRef.attachment = 0;
        finalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 1: Resolve image(MultiSample), source of multisample operation
        VkAttachmentDescription multiAttachment{};
		multiAttachment.format = mColorFormat;
        multiAttachment.samples = mDevice->getMaxUsableSampleCount();
        multiAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        multiAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        multiAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        multiAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        multiAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        multiAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // The final layout of the multisample image is color attachment optimal
        mRenderPass->addAttachment(multiAttachment);

        VkAttachmentReference multiAttachmentRef{};
        multiAttachmentRef.attachment = 1; // The multisample image is the second attachment
        multiAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // The layout of the multisample image is color attachment optimal


        // 3: Depth attachment
        // Description of depth canvas
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = Wrapper::Image::findDepthFormat(mDevice);
        depthAttachment.samples = mDevice->getMaxUsableSampleCount();
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        mRenderPass->addAttachment(depthAttachment);

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 2;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



        // Create subpass
        Wrapper::SubPass subpass{};
        subpass.addColorAttachmentReference(multiAttachmentRef);
        subpass.setDepthStencilAttachmentReference(depthAttachmentRef);
        subpass.setResolveAttachmentReference(finalAttachmentRef);
        subpass.buildSubPassDescription();

        mRenderPass->addSubpass(subpass);

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        mRenderPass->addDependency(dependency);
        mRenderPass->buildRenderPass();
    }

    void OffscreenRenderTarget::createFramebuffer() {
		// Create framebuffers for the offscreen render target images
        mOffScreenFramebuffers.resize(mImageCount); // Resize the swap chain framebuffers vector to hold the framebuffers
        for (size_t i = 0; i < mImageCount; i++) {
            // FrameBuffer is a collection of attachments, for eaxample, n color attachments, 1 depth attachment forms a framebuffer
            // These attachments are packed into a framebuffer to send into pipeline
            // Create a framebuffer for each swap chain image
            // Be careful of the order
            std::array<VkImageView, 3> attachments{
				mRenderTargetImages[i]->getImageView(), // Render target image view for the framebuffer, serves as a texture for the next renderpass
                mMultisampleImages[i]->getImageView(), // Multisample image view for the framebuffer
                mDepthImages[i]->getImageView()
            }; // Attachments for the framebuffer
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = mRenderPass->getRenderPass(); // Render pass for the framebuffer
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // Number of attachments in the framebuffer
            framebufferInfo.pAttachments = attachments.data(); // Attachments for the framebuffer
            framebufferInfo.width = mWidth; // Width of the framebuffer
            framebufferInfo.height = mHeight; // Height of the framebuffer
            framebufferInfo.layers = 1; // Number of layers in the framebuffer
            if (vkCreateFramebuffer(mDevice->getDevice(), &framebufferInfo, nullptr, &mOffScreenFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Error: Failed to create offscreen framebuffer!");
            }
        }
    }

    VkCommandBuffer OffscreenRenderTarget::beginRendering(VkCommandBuffer cmd) {
        //TODO: 
        return nullptr;
    }

    void OffscreenRenderTarget::setClearColor(int idx, float r, float g, float b, float a) {
        mClearValues[idx].color = { r,g,b,a };
    }
    void OffscreenRenderTarget::setClearDepth(float depth, uint32_t stencil) {
        mClearValues[mDepthBufferIndex].depthStencil = { depth, stencil };
    }
}
