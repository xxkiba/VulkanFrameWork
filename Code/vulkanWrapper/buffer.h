#pragma once

#include "../base.h"
#include "device.h"
#include "commandBuffer.h"
#include "commandPool.h"

namespace FF::Wrapper {
	class Buffer {
	public:
		using Ptr = std::shared_ptr<Buffer>;
		static Ptr create(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
			return std::make_shared<Buffer>(device, size, usage, properties);
		}

		static Ptr createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData);
		static Ptr createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData);
		static Ptr createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData = nullptr);
		static Ptr createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData = nullptr);


		Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~Buffer();

		//change memory by Mapping, suitable for Host visible memory
		void updateBufferByMap(const void* data, VkDeviceSize size);
		//If memory is Local optimal, should create StageBuffer, first copy to stage buffer, then copy to this buffer'
		void updateBufferByStage(void* data, VkDeviceSize size);
		void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);
		uint32_t findMemoryType(Device::Ptr device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

		[[nodiscard]] VkMemoryPropertyFlags getProperties() const { return mProperties; }

		[[nodiscard]] VkBufferUsageFlags getUsage() const { return mUsage; }
		[[nodiscard]] VkDeviceSize getSize() const { return mSize; }

		[[nodiscard]] VkBuffer getBuffer() const { return mBuffer; }
		[[nodiscard]] VkDeviceMemory getMemory() const { return mMemory; }
		[[nodiscard]] const VkDescriptorBufferInfo& getBufferInfo() const { return mBufferInfo; }

	private:
		VkBuffer mBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory mMemory{ VK_NULL_HANDLE };
		Device::Ptr mDevice;
		VkDeviceSize mSize{ 0 };
		VkBufferUsageFlags mUsage{ 0 };
		VkMemoryPropertyFlags mProperties{ 0 };
		VkDescriptorBufferInfo mBufferInfo{};
	};
}