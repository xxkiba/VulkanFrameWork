#pragma once
#include "buffer.h"
#include "../texture/texture.h"

// we need to konw how many uniform buffers we need to create and the size of each uniform buffer, how to bind them to the descriptor set
namespace FF::Wrapper {
	struct UniformParameter {
		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() {
			return std::make_shared<UniformParameter>();
		}
		size_t mSize{ 0 }; // size of the buffer
		uint32_t mBinding{ 0 }; // binding index
		uint32_t mCount{ 1 }; // number of descriptors,might be more than one, count refers to the number of descriptors in the array,need to use indexedDescriptor
		VkDescriptorType mDescriptorType{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }; // type of the descriptor
		VkShaderStageFlags mStageFlags{ VK_SHADER_STAGE_ALL }; // stage flags, which shader stages will use this descriptor

		std::vector<Buffer::Ptr> mBuffers{};
		Texture::Ptr mTexture{ nullptr };
	};
}
