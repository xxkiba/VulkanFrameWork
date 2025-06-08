#include "descriptorSet.h"

namespace FF::Wrapper {
	DescriptorSet::DescriptorSet(const Device::Ptr& device, const std::vector<UniformParameter::Ptr> params, const DescriptorSetLayout::Ptr& layout, const DescriptorPool::Ptr& pool, int frameCount)
		: mDevice(device) {
		mDescriptorSets.resize(frameCount);
		std::vector<VkDescriptorSetLayout> layouts(frameCount, layout->getLayout());
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool->getDescriptorPool();
		allocInfo.descriptorSetCount = frameCount;
		allocInfo.pSetLayouts = layouts.data();
		
		if (vkAllocateDescriptorSets(mDevice->getDevice(), &allocInfo, mDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < mDescriptorSets.size(); i++) {
			//for each descriptor set, we need to put params info into the descriptor set
			std::vector<VkWriteDescriptorSet> descriptorWrites;
			for (const auto& param : params) {

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = mDescriptorSets[i];
				descriptorWrite.dstBinding = param->mBinding;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = param->mDescriptorType;
				descriptorWrite.descriptorCount = param->mCount;
				if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					descriptorWrite.pImageInfo = &param->mTexture->getImageInfo();
				}
				else if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					descriptorWrite.pBufferInfo = &param->mBuffers[i]->getBufferInfo();
				}
				descriptorWrites.push_back(descriptorWrite);
				
			}
			vkUpdateDescriptorSets(mDevice->getDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
	DescriptorSet::~DescriptorSet() {
		
	}
}