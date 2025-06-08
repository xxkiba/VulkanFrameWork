#pragma once
#include "../base.h"
#include "device.h"
#include "description.h"
#include "descriptorSetLayout.h"
#include "descriptorPool.h"

namespace FF::Wrapper {
	/*
	* For each model, we need to create a descriptor set to bind the uniform buffer to the pipeline, bind position is in command buffer
	* For each DescriptorSet, it contains vpMatrix, modelMatrix, and other uniform buffers, and also the texture, binding size.
	* 
	* Because of the existance of swapchain, we need to create a descriptor set for each frame.
	*/
	class DescriptorSet {
	public:
		using Ptr = std::shared_ptr<DescriptorSet>;
		static Ptr create(const Device::Ptr& device, const std::vector<UniformParameter::Ptr> params, const DescriptorSetLayout::Ptr& layout, const DescriptorPool::Ptr& pool, int frameCount) {
			return std::make_shared<DescriptorSet>(device, params, layout,  pool,  frameCount);
		}

		DescriptorSet(const Device::Ptr& device,const std::vector<UniformParameter::Ptr> params,const DescriptorSetLayout::Ptr &layout,const DescriptorPool::Ptr &pool,int frameCount);
		~DescriptorSet();
		[[nodiscard]] auto getDescriptorSet(int frameCount) const {
			return mDescriptorSets[frameCount];
		}

		void updateBuffer(const UniformParameter::Ptr& param, const Buffer::Ptr& buffer);

	private:
		std::vector<VkDescriptorSet> mDescriptorSets{};
		Device::Ptr mDevice{ nullptr };
	};
}