#include "offscreenRenderTarget.h"

namespace FF {
    OffscreenRenderTarget::OffscreenRenderTarget(
        const Wrapper::Device::Ptr& device,
        const Wrapper::CommandPool::Ptr& commandPool,
        uint32_t width, uint32_t height,
        int colorCount,
        VkFormat colorFormat,
        VkFormat depthFormat)
        : mDevice(device), mCommandPool(commandPool), mWidth(width), mHeight(height),
        mColorBufferCount(colorCount), mColorFormat(colorFormat), mDepthFormat(depthFormat)
    {
        createAttachments();
        createRenderPass();
        createFramebuffer();
    }

    OffscreenRenderTarget::~OffscreenRenderTarget() {
        cleanup();
    }

    void OffscreenRenderTarget::cleanup() {
        if (mFramebuffer) {
            vkDestroyFramebuffer(mDevice->getDevice(), mFramebuffer, nullptr);
            mFramebuffer = VK_NULL_HANDLE;
        }
		mRenderPass.reset();
        mColorAttachments.clear();
        mDepthAttachment.reset();
        mClearValues.clear();
    }

    void OffscreenRenderTarget::createAttachments() {
        mColorAttachments.clear();
        mClearValues.clear();

        for (int i = 0; i < mColorBufferCount; ++i) {
            auto colorTex = Wrapper::Image::create(
                mDevice,
                mWidth, mHeight,
                mColorFormat,
                VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
            mColorAttachments.push_back(colorTex);

            VkClearValue cv{};
            cv.color = { 0.0f, 0.0f, 0.0f, 0.0f };
            mClearValues.push_back(cv);
        }

        mDepthAttachment = Wrapper::Image::createDepthImage(mDevice,mWidth,mHeight);

        VkClearValue cv{};
        cv.depthStencil = { 1.0f, 0 };
        mClearValues.push_back(cv);

        mDepthBufferIndex = mColorBufferCount;
    }

    void OffscreenRenderTarget::createRenderPass() {
        std::vector<VkAttachmentDescription> attachments(mColorBufferCount + 1);
        std::vector<VkAttachmentReference> colorAttachmentRefs(mColorBufferCount);
        // Create subpass
        Wrapper::SubPass subpass{};
        int attachmentIndex = 0;
        for (int i = 0; i < mColorBufferCount; ++i, ++attachmentIndex) {
            attachments[attachmentIndex] = {
                0,
                mColorFormat,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
            colorAttachmentRefs[i] = { uint32_t(attachmentIndex), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			mRenderPass->addAttachment(attachments[attachmentIndex]);
			subpass.addColorAttachmentReference(colorAttachmentRefs[i]);
        }
        attachments[attachmentIndex] = {
            0,
            mDepthFormat,
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
		mRenderPass->addAttachment(attachments[attachmentIndex]);
        VkAttachmentReference depthAttachmentRef = { uint32_t(attachmentIndex), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        subpass.setDepthStencilAttachmentReference(depthAttachmentRef);


        //VkSubpassDescription subpass{};
        //subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        //subpass.colorAttachmentCount = uint32_t(colorAttachmentRefs.size());
        //subpass.pColorAttachments = colorAttachmentRefs.data();
        //subpass.pDepthStencilAttachment = &depthAttachmentRef;

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
        std::vector<VkImageView> attachments;
        for (auto& tex : mColorAttachments) attachments.push_back(tex->getImageView());
        attachments.push_back(mDepthAttachment->getImageView());

        VkFramebufferCreateInfo fbci{};
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.pAttachments = attachments.data();
        fbci.attachmentCount = attachments.size();
        fbci.width = mWidth;
        fbci.height = mHeight;
        fbci.layers = 1;
        fbci.renderPass = mRenderPass ->getRenderPass();
        vkCreateFramebuffer(mDevice->getDevice(), &fbci, nullptr, &mFramebuffer);
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
