#include "OffscreenPipeline.h"

namespace FF {

    OffscreenPipeline::OffscreenPipeline(const Wrapper::Device::Ptr& device)
        : mDevice(device), mPipeline(nullptr), mWidth(0), mHeight(0) {
    }

    OffscreenPipeline::~OffscreenPipeline() {
        mPipeline.reset();
    }

    void OffscreenPipeline::build(
        const Wrapper::RenderPass::Ptr& renderPass,
        uint32_t width, uint32_t height,
        const std::string& vertexShaderFile, const std::string& fragShaderFile,
        const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
        const std::vector<VkVertexInputBindingDescription>& bindingDes,
        const std::vector<VkVertexInputAttributeDescription>& attributeDes,
        const std::vector<VkPushConstantRange>* pushConstantRanges,
        VkSampleCountFlagBits sampleCount)
    {
        mWidth = width;
        mHeight = height;

        mPipeline = Wrapper::Pipeline::create(mDevice, renderPass);

        // 1. viewport/scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(mHeight);
        viewport.width = static_cast<float>(mWidth);
        viewport.height = -static_cast<float>(mHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        mPipeline->setViewports({ viewport });

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { mWidth, mHeight };
        mPipeline->setScissors({ scissor });

        // 2. shader group
        std::vector<Wrapper::Shader::Ptr> shaderGroup;
        shaderGroup.push_back(Wrapper::Shader::create(mDevice, vertexShaderFile, VK_SHADER_STAGE_VERTEX_BIT, "main"));
        shaderGroup.push_back(Wrapper::Shader::create(mDevice, fragShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT, "main"));
        mPipeline->setShaderGroup(shaderGroup);

        // 3. viewport/scissor state
        mPipeline->mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        mPipeline->mViewportState.viewportCount = static_cast<uint32_t>(mPipeline->mViewports.size());
        mPipeline->mViewportState.pViewports = mPipeline->mViewports.data();
        mPipeline->mViewportState.scissorCount = static_cast<uint32_t>(mPipeline->mScissors.size());
        mPipeline->mViewportState.pScissors = mPipeline->mScissors.data();

        // 4. dynamic state 
        mPipeline->mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        mPipeline->mDynamicState.dynamicStateCount = 0;
        mPipeline->mDynamicState.pDynamicStates = nullptr;

        // 5. vertex input
        mPipeline->mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        mPipeline->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDes.size());
        mPipeline->mVertexInputState.pVertexBindingDescriptions = bindingDes.data();
        mPipeline->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDes.size());
        mPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

        // 6. assembly/raster/multisample/depth/blend
        mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

        mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;
        mPipeline->mRasterState.lineWidth = 1.0f;
        mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
        mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        mPipeline->mRasterState.rasterizerDiscardEnable = VK_FALSE;
        mPipeline->mRasterState.depthClampEnable = VK_FALSE;
        mPipeline->mRasterState.depthBiasEnable = VK_FALSE;

        mPipeline->mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        mPipeline->mMultisampleState.sampleShadingEnable = VK_FALSE;
        mPipeline->mMultisampleState.rasterizationSamples = sampleCount;
        mPipeline->mMultisampleState.minSampleShading = 1.0f;
        mPipeline->mMultisampleState.pSampleMask = nullptr;
        mPipeline->mMultisampleState.alphaToCoverageEnable = VK_FALSE;
        mPipeline->mMultisampleState.alphaToOneEnable = VK_FALSE;

        mPipeline->mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        mPipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
        mPipeline->mDepthStencilState.depthWriteEnable = VK_TRUE;
        mPipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        mPipeline->mDepthStencilState.depthBoundsTestEnable = VK_FALSE;
        mPipeline->mDepthStencilState.stencilTestEnable = VK_FALSE;
        mPipeline->mDepthStencilState.front = {};
        mPipeline->mDepthStencilState.back = {};
        mPipeline->mDepthStencilState.minDepthBounds = 0.0f;
        mPipeline->mDepthStencilState.maxDepthBounds = 1.0f;

        // Blend
        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.blendEnable = VK_FALSE;
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        mPipeline->pushBlendAttachment(blendAttachment);

        mPipeline->mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        mPipeline->mBlendState.logicOpEnable = VK_FALSE;
        mPipeline->mBlendState.logicOp = VK_LOGIC_OP_COPY;
        mPipeline->mBlendState.attachmentCount = static_cast<uint32_t>(mPipeline->mBlendAttachmentStates.size());
        mPipeline->mBlendState.pAttachments = mPipeline->mBlendAttachmentStates.data();
        mPipeline->mBlendState.blendConstants[0] = 0.0f;
        mPipeline->mBlendState.blendConstants[1] = 0.0f;
        mPipeline->mBlendState.blendConstants[2] = 0.0f;
        mPipeline->mBlendState.blendConstants[3] = 0.0f;

        // pipeline layout
        mPipeline->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        mPipeline->mPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        mPipeline->mPipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        if (pushConstantRanges && !pushConstantRanges->empty()) {
            mPipeline->mPipelineLayoutInfo.pushConstantRangeCount = uint32_t(pushConstantRanges->size());
            mPipeline->mPipelineLayoutInfo.pPushConstantRanges = pushConstantRanges->data();
        }
        else {
            mPipeline->mPipelineLayoutInfo.pushConstantRangeCount = 0;
            mPipeline->mPipelineLayoutInfo.pPushConstantRanges = nullptr;
        }

        mPipeline->build();
    }

}

