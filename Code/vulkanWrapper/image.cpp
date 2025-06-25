#include "image.h"

namespace FF::Wrapper {

	Image::Ptr Image::createDepthImage(const Device::Ptr& device, const int& width, const int& height) {
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		VkFormat depthFormat = findSupportedFormat(
			device,
			depthFormats,
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		return create(
			device,
			width, height,
			depthFormat,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			device->getMaxUsableSampleCount(),
			VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	Image::Ptr Image::createRenderTargetImage(const Device::Ptr& device, const int& width, const int& height, VkFormat format) {
		return create(
			device,
			width, height,
			format,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,//VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, when a image has this flag, vulkan would do lazy create, which means it would not allocate memory until the first time it is used
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,//If transient is used, need to use VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
			device->getMaxUsableSampleCount(),
			VK_IMAGE_ASPECT_COLOR_BIT);
	}

	Image::Image(const Device::Ptr& device,
		const int& width,const int& height,
		const VkFormat format,
		const VkImageType& imageType,
		const VkImageTiling& tiling,
		const VkImageUsageFlags& usage,
		const VkMemoryPropertyFlags& properties,
		const VkSampleCountFlagBits& sample,
		const VkImageAspectFlags &aspectFlags,
		const bool& isCubeMap,
		const int mipmapLevels):mDevice(device),mFormat(format),mImageLayout(VK_IMAGE_LAYOUT_UNDEFINED){
		if (width == 0 || height == 0) {
			throw std::runtime_error("Image width or height is zero!");
		}
		mExtent.width = width;
		mExtent.height = height;
		mExtent.depth = 1;
		mSize = width * height * 4;
		mAlignment = 4;
		mOffset = 0;
		mUsage = usage;
		mProperties = properties;
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = imageType;
		imageInfo.extent = mExtent;
		imageInfo.mipLevels = mipmapLevels;
		imageInfo.arrayLayers = isCubeMap ? 6 : 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = sample;
		imageInfo.flags = isCubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT:0;
		if (vkCreateImage(mDevice->getDevice(), &imageInfo, nullptr, &mImage) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image!");
		}

		//Allocate memory space
		VkMemoryRequirements memRequirements{};
		vkGetImageMemoryRequirements(mDevice->getDevice(), mImage, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(mDevice, memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mImageMemory) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to allocate image memory!");
		}

		vkBindImageMemory(mDevice->getDevice(), mImage, mImageMemory, 0);

		//Create image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = mImage;
        viewInfo.viewType = isCubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : ((imageType & VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D);
		viewInfo.format = format;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipmapLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = isCubeMap ? 6 : 1;

		if (vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mImageView) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create image view!");
		}


	}

	VkFormat Image::findSupportedFormat(
		const Device::Ptr& device,
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features) {
		for (const auto& format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(device->getPhysicalDevice(), format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("Error: failed to find supported format!");
	}

	bool Image::hasStencilComponent(VkFormat format) {
		return mFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || mFormat == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	Image::Image::~Image() {
		destroyImageView();
		destroyMemory();
		destroyImage();
	}

	void Image::setImageLayout(
		VkImageLayout newLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		VkImageSubresourceRange subresourceRange,
		const CommandPool::Ptr& commandPool,
		const CommandBuffer::Ptr& inCommandBUffer) {

		
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = mImageLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = mImage;
		barrier.subresourceRange = subresourceRange;

		switch (mImageLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		default:
			break;
		}

		switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
			//As a texture, the source has two options: one is from staging buffer, the other is from cpu copy.
			if (barrier.srcAccessMask == 0) {
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
		}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		default:
			break;

		}
		

		if (inCommandBUffer == nullptr) {
			auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
			commandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			commandBuffer->transferImageLayout(barrier, srcStageMask, dstStageMask);
			commandBuffer->endCommandBuffer();

			commandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
		}
		else {
			inCommandBUffer->transferImageLayout(barrier, srcStageMask, dstStageMask);
		}

		//commandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());
		mImageLayout = newLayout;
	}

	void Image::fillImageData(size_t size, const void* pData, const CommandPool::Ptr& commandPool,const bool& isCubeMap) {
		assert(pData != nullptr);
		assert(size > 0);

		auto stageBuffer = Buffer::createStageBuffer(mDevice, size, (void*)pData);

		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
		commandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->copyBufferToImage(stageBuffer->getBuffer(), mImage, mImageLayout, mExtent.width, mExtent.height,isCubeMap);
		commandBuffer->endCommandBuffer();

		commandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
		//commandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());

	}

	void Image::CopyImageToCubeMap(const CommandPool::Ptr& commandPool, const VkImage& inSrcImage,VkImage inDstCubeMap, size_t inWidth, size_t inHeight, int inFace, int inMipmapLevel) {
		auto commandBuffer = CommandBuffer::create(mDevice, commandPool);
		commandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		commandBuffer->CopyRTImageToCubeMap(inSrcImage, inDstCubeMap, inWidth, inHeight, inFace, inMipmapLevel);
		commandBuffer->endCommandBuffer();
		commandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
		
	}

	void Image::destroyImageView() {
		if (mImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(mDevice->getDevice(), mImageView, nullptr);
			mImageView = VK_NULL_HANDLE;
		}
	}

	void Image::destroyImage() {
		if (mImage != VK_NULL_HANDLE) {
			vkDestroyImage(mDevice->getDevice(), mImage, nullptr);
			mImage = VK_NULL_HANDLE;
		}
	}

	void Image::destroyMemory() {
		if (mImageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(mDevice->getDevice(), mImageMemory, nullptr);
			mImageMemory = VK_NULL_HANDLE;
		}
	}

	uint32_t Image::findMemoryType(Device::Ptr device, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
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