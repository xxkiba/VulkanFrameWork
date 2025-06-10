#include "buffer.h"

namespace FF::Wrapper {

	Buffer::Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		: mDevice(device) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// It is a buffer descriptor, not a gpu buffer
		if (vkCreateBuffer(mDevice->getDevice(), &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create buffer!");
		}

		// Create GPU memory for the buffer
		VkMemoryRequirements memRequirements{};
		vkGetBufferMemoryRequirements(mDevice->getDevice(), mBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(mDevice,memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mMemory) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate buffer memory!");
		}
		vkBindBufferMemory(mDevice->getDevice(), mBuffer, mMemory, 0);

		mBufferInfo.buffer = mBuffer;
		mBufferInfo.offset = 0;
		mBufferInfo.range = size;
	}

	Buffer::~Buffer() {

		if (mBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(mDevice->getDevice(), mBuffer, nullptr);
		}

		if (mMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mDevice->getDevice(), mMemory, nullptr);
		}

	}

	Buffer::Ptr Buffer::createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
		auto buffer = Buffer::create(device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		buffer->updateBufferByStage(pData, size);
		return buffer;
	}

	Buffer::Ptr Buffer::createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
		auto buffer = Buffer::create(device, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		buffer->updateBufferByStage(pData, size);
		return buffer;
	}

	Buffer::Ptr Buffer::createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
		auto buffer = Buffer::create(device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (pData != nullptr) {
			buffer->updateBufferByMap(pData, size);
		}
		
		return buffer;
	}

	Buffer::Ptr Buffer::createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
		auto buffer = Buffer::create(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		if (pData != nullptr) {
			buffer->updateBufferByMap(pData, size);
		}
		return buffer;
	}


	void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size) {
		auto commandPool = CommandPool::create(mDevice);
		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);

		commandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		commandBuffer->copyBufferToBuffer(srcBuffer, dstBuffer, 1, { copyRegion });
		commandBuffer->endCommandBuffer();
		commandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
		commandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());

	}

	void Buffer::updateBufferByMap(const void* data, VkDeviceSize size) {
		void* mappedData;
		vkMapMemory(mDevice->getDevice(), mMemory, 0, size, 0, &mappedData);
		memcpy(mappedData, data, static_cast<size_t>(size));
		vkUnmapMemory(mDevice->getDevice(), mMemory);
	}

	void Buffer::updateBufferByStage(void* data, VkDeviceSize size) {
		// Create a staging buffer
		auto stagingBuffer = Buffer::createStageBuffer(mDevice, size, nullptr);
		stagingBuffer->updateBufferByMap(data, size);
		
		copyBuffer(stagingBuffer->getBuffer(), mBuffer, size);
	}
	uint32_t Buffer::findMemoryType(Device::Ptr device,uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("Error: failed to find suitable memory type!");
	}
}