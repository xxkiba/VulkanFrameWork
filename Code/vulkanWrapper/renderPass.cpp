#include "renderPass.h"

namespace FF::Wrapper {

	SubPass::SubPass() {

	}

	SubPass::~SubPass() {

	}

	void SubPass::addInputAttachmentReference(const VkAttachmentReference& ref) {
		mInputAttachmentReferences.push_back(ref);
	}

	void SubPass::addColorAttachmentReference(const VkAttachmentReference& ref) {
		mColorAttachmentReferences.push_back(ref);
	}

	void SubPass::setDepthStencilAttachmentReference(const VkAttachmentReference& ref) {
		mDepthStencilAttachmentReference = ref;
	}

	void SubPass::setResolveAttachmentReference(const VkAttachmentReference& ref) {
		mResolveAttachmentReference = ref;
	}


	void SubPass::buildSubPassDescription() {
		if (mColorAttachmentReferences.empty()) {
			throw std::runtime_error("Color attachment references are empty");
		}

		mSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		mSubpassDescription.inputAttachmentCount = static_cast<uint32_t>(mInputAttachmentReferences.size());
		mSubpassDescription.pInputAttachments = mInputAttachmentReferences.data();

		mSubpassDescription.colorAttachmentCount = static_cast<uint32_t>(mColorAttachmentReferences.size());

		mSubpassDescription.pColorAttachments = mColorAttachmentReferences.data();

		mSubpassDescription.pResolveAttachments = mResolveAttachmentReference.layout == VK_IMAGE_LAYOUT_UNDEFINED ? nullptr : &mResolveAttachmentReference;

		mSubpassDescription.pDepthStencilAttachment = mDepthStencilAttachmentReference.layout == VK_IMAGE_LAYOUT_UNDEFINED ? nullptr:&mDepthStencilAttachmentReference;
	}


	RenderPass::RenderPass(const Device::Ptr& device)
		: mDevice(device) {
	}
	RenderPass::~RenderPass() {
		if (mRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(mDevice->getDevice(), mRenderPass, nullptr);
			mRenderPass = VK_NULL_HANDLE;
		}
		mDevice.reset();
	}
	void RenderPass::addAttachment(const VkAttachmentDescription& attachmentDescription) {
		mAttachmentDescriptions.push_back(attachmentDescription);
	}
	void RenderPass::addSubpass(const SubPass& subpass) {
		mSubPasses.push_back(subpass);
	}
	void RenderPass::addDependency(const VkSubpassDependency& dependency) {
		mDependencies.push_back(dependency);
	}
	void RenderPass::buildRenderPass() {
		if (mSubPasses.empty() || mAttachmentDescriptions.empty() || mDependencies.empty()) {
			throw std::runtime_error("Subpass, attachment descriptions or dependencies are empty");
		}

		//unwrap
		std::vector<VkSubpassDescription> subPasses{};
		for (int i = 0; i < mSubPasses.size(); i++) {
			subPasses.push_back(mSubPasses[i].getSubpassDescription());
		}

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

		renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(mAttachmentDescriptions.size());
		renderPassCreateInfo.pAttachments = mAttachmentDescriptions.data();

		renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subPasses.size());
		renderPassCreateInfo.pSubpasses = subPasses.data();

		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(mDependencies.size());
		renderPassCreateInfo.pDependencies = mDependencies.data();

		if (vkCreateRenderPass(mDevice->getDevice(), &renderPassCreateInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create render pass!");
		}
	}
}