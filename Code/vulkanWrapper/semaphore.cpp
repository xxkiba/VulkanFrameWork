#include "semaphore.h"

namespace FF::Wrapper {
	
	Semaphore::Semaphore(const Device::Ptr& device) {
		mDevice = device;
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(mDevice->getDevice(), &semaphoreInfo, nullptr, &mSemaphore) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create semaphore!");
		}
	}

	Semaphore::~Semaphore() {
		if (mSemaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(mDevice->getDevice(), mSemaphore, nullptr);
			mSemaphore = VK_NULL_HANDLE;
		}
	}
}