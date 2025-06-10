#pragma once

#include "../base.h"
#include "device.h"

namespace FF::Wrapper {
	/*
	*  fence is used to synchronize the command buffer and the GPU, the difference between fence and semaphore is that 
	*  semaphore is used to control the rely relationship of different stages of the pipeline
	*  fence control a batch of  commands for a queue (graphicsQueue), ensuring that all the commands in the queue are completed
	*/
	class Fence {
	public:
		using Ptr = std::shared_ptr<Fence>;
		static Ptr create(const Device::Ptr& device, bool signaled = true) {
			return std::make_shared<Fence>(device,signaled);
		}
		Fence(const Device::Ptr& device,bool signaled = true);
		~Fence();
		void waitForFence(uint64_t timeout = UINT64_MAX);
		// Wait for the fence to be signaled
		void resetFence();


		[[nodiscard]] auto getFence() const { return mFence; }
	private:
		VkFence mFence{ VK_NULL_HANDLE };
		Device::Ptr mDevice{nullptr};
	};
}