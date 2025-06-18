#pragma once
#include "SceneNode.h"

namespace FF {
    class OffscreenSceneNode : public SceneNode {
    public:
        using Ptr = std::shared_ptr<OffscreenSceneNode>;
        static Ptr create() {
            return std::make_shared<OffscreenSceneNode>();
        }
        OffscreenSceneNode();
        ~OffscreenSceneNode();

        // set shader path
        void setShaderFiles(const std::string& vs, const std::string& fs);

        // get shader path
        const std::string& getVertexShaderFile() const;
        const std::string& getFragmentShaderFile() const;

		// get vertex input binding and attribute descriptions, by default from the first model
        const std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions() const;
        const std::vector<VkVertexInputAttributeDescription> getVertexInputAttributeDescriptions() const;

		// get descriptor set layouts from material
        std::vector<VkDescriptorSetLayout> getDescriptorSetLayouts() const;

    private:
        std::string mVertexShaderFile;
        std::string mFragmentShaderFile;
		std::vector<VkVertexInputBindingDescription> mVertexInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> mVertexInputAttributeDescriptions;
    };
}