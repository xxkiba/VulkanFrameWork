#include "commandBuffer.h"

namespace FF::Wrapper {
	CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary)
		: mDevice(device), mCommandPool(commandPool) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = mCommandPool->getCommandPool();
		allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocInfo, &mCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to allocate command buffer!");
		}
	}
	CommandBuffer::~CommandBuffer() { 
		if (mCommandBuffer != VK_NULL_HANDLE) {
			vkFreeCommandBuffers(mDevice->getDevice(), mCommandPool->getCommandPool(), 1, &mCommandBuffer);
			mCommandBuffer = VK_NULL_HANDLE;
		}
		mCommandPool = nullptr;
		mDevice = nullptr;
	}
	void CommandBuffer::beginCommandBuffer(VkCommandBufferUsageFlags flag, const VkCommandBufferInheritanceInfo& inheritance) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flag;
		if (flag & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) {
			beginInfo.pInheritanceInfo = &inheritance;
		}
		if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to begin command buffer!");
		}
	}
	void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, VkSubpassContents subPassContents) {
		vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, subPassContents);
	}
	void CommandBuffer::bindGraphicPipeline(const Pipeline::Ptr& pipeline) {
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());
	}

	void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer>& buffers, uint32_t binding, std::vector<VkDeviceSize> offsets) {
		offsets.resize(buffers.size(), 0);
		vkCmdBindVertexBuffers(mCommandBuffer, binding, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
	}

	void CommandBuffer::bindIndexBuffer(VkBuffer buffer, uint32_t offset, VkIndexType indexType) {
		vkCmdBindIndexBuffer(mCommandBuffer, buffer, offset, indexType);
	}

	void CommandBuffer::bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet &descriptorSet) {
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void CommandBuffer::bindDescriptorSets(const VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) {
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, firstSet, descriptorSetCount, pDescriptorSets, 0, nullptr);
	}

	void CommandBuffer::pushConstants(const VkPipelineLayout layout,VkShaderStageFlagBits flags,uint32_t offset,uint32_t size, void* pData) {
		vkCmdPushConstants(mCommandBuffer,layout,flags,0,sizeof(NVPMatrices),pData);
	}

	void CommandBuffer::draw(uint32_t vertexCount) {
		vkCmdDraw(mCommandBuffer, vertexCount, 1, 0, 0);
	}
	void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
		vkCmdDrawIndexed(mCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	void CommandBuffer::endRenderPass() {
		vkCmdEndRenderPass(mCommandBuffer);
	}
	void CommandBuffer::endCommandBuffer() {
		if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to end command buffer!");
		}
	}
	void CommandBuffer::copyBufferToBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyRegions) {
		vkCmdCopyBuffer(mCommandBuffer, srcBuffer, dstBuffer, copyInfoCount, copyRegions.data());
	}

	void CommandBuffer::copyBufferToImage(const VkBuffer& srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, size_t width, size_t height, bool isCubeMap) {

		if (!isCubeMap) {
			// Single 2D image case
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

			vkCmdCopyBufferToImage(mCommandBuffer, srcBuffer, dstImage, dstImageLayout, 1, &region);
		}
		else {
			// Cubemap
			std::vector<VkBufferImageCopy> regions(6);
			VkDeviceSize layerSize = width * height * 4; // RGBA, assuming 4 bytes per pixel
			// 6 regions needed for each face of the cubemap
			for (uint32_t i = 0; i < 6; ++i) {
				regions[i].bufferOffset = i * layerSize; // Calculate offset for each face
				regions[i].bufferRowLength = 0;
				regions[i].bufferImageHeight = 0;
				regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				regions[i].imageSubresource.mipLevel = 0;
				regions[i].imageSubresource.baseArrayLayer = i; // Set base array layer for each face
				regions[i].imageSubresource.layerCount = 1;
				regions[i].imageOffset = { 0, 0, 0 };
				regions[i].imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
			}
			// Copy all 6 faces of the cubemap
			vkCmdCopyBufferToImage(mCommandBuffer, srcBuffer, dstImage, dstImageLayout,
				static_cast<uint32_t>(regions.size()), regions.data());
		}
	}

	void CommandBuffer::submitCommandBuffer(VkQueue queue, VkFence fence) {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffer;
		if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) {
			throw std::runtime_error("Error: Failed to submit command buffer!");
		}
	}
	void CommandBuffer::waitCommandBuffer(VkQueue queue, VkFence fence) {
			vkQueueWaitIdle(queue);
	}

	void CommandBuffer::transferImageLayout(const VkImageMemoryBarrier &imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
		vkCmdPipelineBarrier(mCommandBuffer, 
			srcStageMask, 
			dstStageMask,
			0, 
			0, nullptr, //memory barrier
			0, nullptr, //buffer barrier
			1, &imageMemoryBarrier);
	}

}