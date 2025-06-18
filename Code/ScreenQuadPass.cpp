#include "ScreenQuadPass.h"
#include "vulkanWrapper/shader.h"

namespace FF {

    ScreenQuadPass::Ptr ScreenQuadPass::create(
        const Wrapper::Device::Ptr& device,
        const OffscreenRenderTarget::Ptr& outputTarget,
        const std::string& vsPath,
        const std::string& fsPath)
    {
        return std::make_shared<ScreenQuadPass>(device, outputTarget, vsPath, fsPath);
    }

    ScreenQuadPass::ScreenQuadPass(
        const Wrapper::Device::Ptr& device,
        const OffscreenRenderTarget::Ptr& outputTarget,
        const std::string& vsPath,
        const std::string& fsPath)
        : mDevice(device),
        mOutputTarget(outputTarget),
        mVertexShaderPath(vsPath),
        mFragmentShaderPath(fsPath)
    {
    }

    ScreenQuadPass::~ScreenQuadPass() {}

    void ScreenQuadPass::setInputImage(VkImageView imageView, VkSampler sampler) {
        mInputImageView = imageView;
        mInputSampler = sampler;
        mBuilt = false;
    }

    void ScreenQuadPass::build() {
        if (mBuilt) return;
        createDescriptor();
        createPipeline();
        mBuilt = true;
    }

    void ScreenQuadPass::createDescriptor() {
        // 1. Layout
        auto samplerParam = Wrapper::UniformParameter::create();
        samplerParam->mBinding = 0;
        samplerParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerParam->mCount = 1;
        samplerParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<Wrapper::UniformParameter::Ptr> params = { samplerParam };
        mDescriptorLayout = Wrapper::DescriptorSetLayout::create(mDevice);
        mDescriptorLayout->build(params);

        mDescriptorPool = Wrapper::DescriptorPool::create(mDevice);
        mDescriptorPool->build(params, 1);

        mDescriptorSet = Wrapper::DescriptorSet::create(mDevice, params, mDescriptorLayout, mDescriptorPool, 1);
    }

    void ScreenQuadPass::createPipeline() {
        mPipeline = Wrapper::Pipeline::create(mDevice, mOutputTarget->getRenderPass());

        // viewport/scissor = output target size
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = float(mOutputTarget->getHeight());
        viewport.width = float(mOutputTarget->getWidth());
        viewport.height = -float(mOutputTarget->getHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        mPipeline->setViewports({ viewport });

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { mOutputTarget->getWidth(), mOutputTarget->getHeight() };
        mPipeline->setScissors({ scissor });

        // shader
        auto vs = Wrapper::Shader::create(mDevice, mVertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT, "main");
        auto fs = Wrapper::Shader::create(mDevice, mFragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT, "main");
        std::vector<Wrapper::Shader::Ptr> shaders = { vs, fs };
        mPipeline->setShaderGroup(shaders);

        // Full screen triangle, no need for vertex buffer
        mPipeline->mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        mPipeline->mVertexInputState.vertexBindingDescriptionCount = 0;
        mPipeline->mVertexInputState.pVertexBindingDescriptions = nullptr;
        mPipeline->mVertexInputState.vertexAttributeDescriptionCount = 0;
        mPipeline->mVertexInputState.pVertexAttributeDescriptions = nullptr;

        // Assembly
        mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

        // Raster/depth/multisample
        mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;
        mPipeline->mRasterState.lineWidth = 1.0f;
        mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
        mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        mPipeline->mRasterState.rasterizerDiscardEnable = VK_FALSE;

        mPipeline->mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        mPipeline->mMultisampleState.sampleShadingEnable = VK_FALSE;
        mPipeline->mMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        mPipeline->mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        mPipeline->mDepthStencilState.depthTestEnable = VK_FALSE;
        mPipeline->mDepthStencilState.depthWriteEnable = VK_FALSE;

        // Blend
        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.blendEnable = VK_FALSE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        mPipeline->pushBlendAttachment(blendAttachment);

        mPipeline->mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        mPipeline->mBlendState.attachmentCount = uint32_t(mPipeline->mBlendAttachmentStates.size());
        mPipeline->mBlendState.pAttachments = mPipeline->mBlendAttachmentStates.data();

        // Pipeline Layout
        mPipeline->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        VkDescriptorSetLayout layout = mDescriptorLayout->getLayout();
        mPipeline->mPipelineLayoutInfo.setLayoutCount = 1;
        mPipeline->mPipelineLayoutInfo.pSetLayouts = &layout;

        mPipeline->mPipelineLayoutInfo.pushConstantRangeCount = 0;
        mPipeline->mPipelineLayoutInfo.pPushConstantRanges = nullptr;

        mPipeline->build();
    }

    void ScreenQuadPass::render(const Wrapper::CommandBuffer::Ptr& cmdBuf,const Wrapper::RenderPass::Ptr& renderPass, VkFramebuffer& frameBuffer) {
        if (!mBuilt) build();

        // begin render pass
        VkRenderPassBeginInfo rpbi{};
        rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpbi.renderPass = renderPass->getRenderPass();
		rpbi.framebuffer = frameBuffer;
        rpbi.renderArea.offset = { 0, 0 };
        rpbi.renderArea.extent = { mOutputTarget->getWidth(), mOutputTarget->getHeight() };
        std::vector<VkClearValue> clearValues(1);
        clearValues[0].color = { 0.0f,0.0f,0.0f,0.0f };
        rpbi.clearValueCount = clearValues.size();
        rpbi.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmdBuf->getCommandBuffer(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline
        vkCmdBindPipeline(cmdBuf->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getPipeline());

        // Bind descriptor
        VkDescriptorSet ds = mDescriptorSet->getDescriptorSet(0); // 1 frame by default
        vkCmdBindDescriptorSets(cmdBuf->getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getPipelineLayout(), 0, 1, &ds, 0, nullptr);

		// Full screen triangle
        vkCmdDraw(cmdBuf->getCommandBuffer(), 3, 1, 0, 0);

        vkCmdEndRenderPass(cmdBuf->getCommandBuffer());
    }

} // namespace FF
