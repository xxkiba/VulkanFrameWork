#pragma once


#include "../base.h"
#include "device.h"
#include "shader.h"
#include "renderPass.h"

namespace FF::Wrapper {
	class Pipeline {
	public:
		using Ptr = std::shared_ptr<Pipeline>;
		static Ptr create(const Device::Ptr& device,const RenderPass::Ptr& renderPass) {
			return std::make_shared<Pipeline>(device,renderPass);
		}
		Pipeline(const Device::Ptr& device,const RenderPass::Ptr& renderPass);
		~Pipeline();
		[[nodiscard]] VkPipeline getPipeline() const { return mPipeline; }
		[[nodiscard]] VkPipelineLayout getPipelineLayout() const { return mLayout; }
		//[[nodiscard]] const RenderPass::Ptr& getRenderPass() const { return mRenderPass; }
		//[[nodiscard]] const std::vector<Shader::Ptr>& getShaders() const { return mShaders; }
		void setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup);
		void inline setViewports(const std::vector<VkViewport>& viewports) { mViewports = viewports; }
		void inline setScissors(const std::vector<VkRect2D>& scissors) { mScissors = scissors; }

		void pushBlendAttachment(const VkPipelineColorBlendAttachmentState& blendAttachment) {
			mBlendAttachmentStates.push_back(blendAttachment);
		}

		void build();
	public:
		VkPipelineVertexInputStateCreateInfo mVertexInputState{};
		VkPipelineInputAssemblyStateCreateInfo mAssemblyState{};
		VkPipelineViewportStateCreateInfo mViewportState{};
		VkPipelineRasterizationStateCreateInfo mRasterState{};
		VkPipelineDynamicStateCreateInfo mDynamicState{};
		VkPipelineMultisampleStateCreateInfo mMultisampleState{};
		std::vector<VkPipelineColorBlendAttachmentState>  mBlendAttachmentStates{};
		VkPipelineColorBlendStateCreateInfo mBlendState{};
		VkPipelineDepthStencilStateCreateInfo mDepthStencilState{};
		VkPipelineLayoutCreateInfo mPipelineLayoutInfo{};
		std::vector<VkViewport> mViewports{};
		std::vector<VkRect2D> mScissors{};
		std::vector<VkDynamicState> dynamicStates = {};

		//TODO: render pass and subpass

	private:
		VkPipeline mPipeline{ VK_NULL_HANDLE };
		VkPipelineLayout mLayout{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
		RenderPass::Ptr mRenderPass{ nullptr };
		std::vector<Shader::Ptr> mShaders{};
	};
}