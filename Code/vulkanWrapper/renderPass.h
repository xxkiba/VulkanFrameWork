#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {
	/*
	* 1 attachment
	* VkAttachmentDescription: Describe a color or depth/stencil attachment, not a real attachment, but a description
	* VkAttachmentDescription1 VkAttachmentDescription2 VkAttachmentDescription3 VkAttachmentDescription4(depth/stencil) defines what parameters we need to receive
	* VkAttachmentReference Specify the attachment index and layout in the subpass, expected format
	* VkSubpassDescription: Describe the subpass, not a real subpass, but a description
	* VkSubpassDependency: Describe the dependency between subpasses
	*/

	class SubPass {
	public:
		SubPass();
		~SubPass();
		void addInputAttachmentReference(const VkAttachmentReference& ref);
		void addColorAttachmentReference(const VkAttachmentReference& ref);
		void setDepthStencilAttachmentReference(const VkAttachmentReference& ref);
		void setResolveAttachmentReference(const VkAttachmentReference& ref);

		void buildSubPassDescription();

		
		[[nodiscard]] VkSubpassDescription getSubpassDescription() const { return mSubpassDescription; }

	private:
		VkSubpassDescription mSubpassDescription{};
		std::vector<VkAttachmentReference> mInputAttachmentReferences{};
		std::vector<VkAttachmentReference> mColorAttachmentReferences{};
		VkAttachmentReference mDepthStencilAttachmentReference{};
		VkAttachmentReference mResolveAttachmentReference{};
	};

	class RenderPass {
	public:
		using Ptr = std::shared_ptr<RenderPass>;
		static Ptr create(const Device::Ptr& device) {
			return std::make_shared<RenderPass>(device);
		}
		RenderPass(const Device::Ptr& device);
		~RenderPass();
		void addAttachment(const VkAttachmentDescription& attachmentDescription);
		void addSubpass(const SubPass& subpass);
		void addDependency(const VkSubpassDependency& dependency);
		void buildRenderPass();
		VkRenderPass getRenderPass() const { return mRenderPass; }
	private:
		Device::Ptr mDevice;
		VkRenderPass mRenderPass{ VK_NULL_HANDLE };
		std::vector<VkAttachmentDescription> mAttachmentDescriptions{};

		std::vector<SubPass> mSubPasses{};
		std::vector<VkSubpassDependency> mDependencies;

	};
}