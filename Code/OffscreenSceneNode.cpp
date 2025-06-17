#include "offscreenSceneNode.h"

namespace FF {
    OffscreenSceneNode::OffscreenSceneNode()
        : SceneNode()
    {
        mVertexShaderFile = "";
        mFragmentShaderFile = "";
    }
    OffscreenSceneNode::~OffscreenSceneNode() {}

    void OffscreenSceneNode::setShaderFiles(const std::string& vs, const std::string& fs) {
        mVertexShaderFile = vs;
        mFragmentShaderFile = fs;
    }
    const std::string& OffscreenSceneNode::getVertexShaderFile() const {
        return mVertexShaderFile;
    }
    const std::string& OffscreenSceneNode::getFragmentShaderFile() const {
        return mFragmentShaderFile;
    }

    const std::vector<VkVertexInputBindingDescription>& OffscreenSceneNode::getVertexInputBindingDescriptions() const {
		if (mModels.empty()) {
			throw std::runtime_error("No models available to get vertex input binding descriptions.");
		}
        return mModels[0]->getVertexInputBindingDescriptions();
    }
    const std::vector<VkVertexInputAttributeDescription>& OffscreenSceneNode::getVertexInputAttributeDescriptions() const {
		if (mModels.empty()) {
			throw std::runtime_error("No models available to get vertex input attribute descriptions.");
		}
        return mModels[0]->getAttributeDescriptions();
    }

    std::vector<VkDescriptorSetLayout> OffscreenSceneNode::getDescriptorSetLayouts() const {
        // ¥”materialªÒ»°
        std::vector<VkDescriptorSetLayout> layouts;
        if (mUniformManager && mUniformManager->getDescriptorLayout()) {
            layouts.push_back(mUniformManager->getDescriptorLayout()->getLayout());
        }
        if (mMaterial && mMaterial->getDescriptorLayout()) {
            layouts.push_back(mMaterial->getDescriptorLayout()->getLayout());
        }
        return layouts;
    }
}
