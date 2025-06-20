#pragma once

#include "../base.h"
#include "device.h"
#include "commandBuffer.h"
#include "commandPool.h"

namespace FF::Wrapper {
	/*
	* Sampler is used to sample the texture, it contains the filter, address mode, and other parameters
	*/
	class Sampler {
	public:
		using Ptr = std::shared_ptr<Sampler>;
		static Ptr create(const Device::Ptr& device,bool isCubeMap = false) {
			return std::make_shared<Sampler>(device,isCubeMap);
		}
		Sampler(const Device::Ptr& device,bool isCubeMap = false);
		~Sampler();

		void createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode, bool isCubeMap = false);
		[[nodiscard]] VkSampler getSampler() const { return mSampler; }
	private:
		VkSampler mSampler{ VK_NULL_HANDLE };
		Device::Ptr mDevice{ nullptr };
	};
}