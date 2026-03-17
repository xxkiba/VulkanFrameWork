#pragma once

#include "../base.h"
#include "device.h"
#include <optional>

namespace FF::Wrapper {

    class SubPass {
    public:
        SubPass() = default;
        ~SubPass() = default;

        void addInputAttachmentReference(const VkAttachmentReference& ref) {
            mInputAttachmentReferences.push_back(ref);
        }

        void addColorAttachmentReference(const VkAttachmentReference& ref) {
            mColorAttachmentReferences.push_back(ref);
        }

        void setDepthStencilAttachmentReference(const VkAttachmentReference& ref) {
            mDepthStencilAttachmentReference = ref;
        }

        void setResolveAttachmentReference(const VkAttachmentReference& ref) {
            mResolveAttachmentReference = ref;
        }

        void validate() const;

    private:
        friend class RenderPass;

        std::vector<VkAttachmentReference> mInputAttachmentReferences{};
        std::vector<VkAttachmentReference> mColorAttachmentReferences{};
        std::optional<VkAttachmentReference> mDepthStencilAttachmentReference{};
        std::optional<VkAttachmentReference> mResolveAttachmentReference{};
    };

    class RenderPass {
    public:
        using Ptr = std::shared_ptr<RenderPass>;
        static Ptr create(const Device::Ptr& device) {
            return std::make_shared<RenderPass>(device);
        }

        explicit RenderPass(const Device::Ptr& device);
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
        std::vector<VkSubpassDependency> mDependencies{};
    };

} // namespace FF::Wrapper