#pragma once
#include "../base.h"
#include "device.h"
#include "commandBuffer.h"
#include "commandPool.h"
#include "buffer.h"

namespace FF::Wrapper {
	/*
	* if we want to use a image as a texture, we need to tranform format from undefinedLayout to TransferDst, and then transform to shaderreadonly after data copy
	*/
	class Image {
	public:
		using Ptr = std::shared_ptr<Image>;
		// static tool function
		static Image::Ptr createDepthImage(
			const Device::Ptr &device,
			const int&width,
			const int& height);

		static Image::Ptr createRenderTargetImage(
			const Device::Ptr& device,
			const int& width,
			const int& height,
			VkFormat format);

		static Image::Ptr createFromFile(
			const Device::Ptr& device,
			const CommandPool::Ptr& commandPool,
			const std::string& filePath,
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
			bool flipVertically = false
		);


		static VkFormat findDepthFormat(const Device::Ptr &device) {
			std::vector<VkFormat> depthFormats = {
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT
			};
			return findSupportedFormat(
				device,
				depthFormats,
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		}



		static VkFormat findSupportedFormat(
			const Device::Ptr& device,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features);

		bool hasStencilComponent(VkFormat format);


	public:
		
		static Ptr create(const Device::Ptr& device,
			const int& width, 
			const int& height,
			const VkFormat format,
			const VkImageType& imageType,
			const VkImageTiling& tiling,
			const VkImageUsageFlags& usage,
			const VkMemoryPropertyFlags& properties,
			const VkSampleCountFlagBits& sample,
			const VkImageAspectFlags& aspectFlag,
			const bool& isCubeMap = false,
			const int mimapLevels = 1) {
			return std::make_shared<Image>(device, width, height, format, imageType, tiling, usage, properties,sample, aspectFlag, isCubeMap,mimapLevels);
		}
		Image(const Device::Ptr& device, 
			const int& width,
			const int& height,
			const VkFormat format,
			const VkImageType& imageType,
			const VkImageTiling& tiling, 
			const VkImageUsageFlags& usage,
			const VkMemoryPropertyFlags& properties, 
			const VkSampleCountFlagBits& sample,
			const VkImageAspectFlags& aspectFlag,
			const bool& isCubeMap = false,
			const int mipmapLevels = 1);
		~Image();
		void createImageView(VkImageViewType viewType);
		void destroyImageView();
		void destroyImage();
		void destroyMemory();

		[[nodiscard]] VkImageView getImageView() const { return mImageView; }
		[[nodiscard]] VkImage getImage() const { return mImage; }
		[[nodiscard]] auto getLayout() const { return mImageLayout; }
		[[nodiscard]] auto getExtent() const { return mExtent; }
		[[nodiscard]] auto getWidth() const { return mExtent.width; }
		[[nodiscard]] auto getHeight() const { return mExtent.height; }
		[[nodiscard]] auto getDepth() const { return mExtent.depth; }
		[[nodiscard]] auto getFormat() const { return mFormat; }
		[[nodiscard]] auto getSize() const { return mSize; }
		[[nodiscard]] auto getAlignment() const { return mAlignment; }
		[[nodiscard]] auto getOffset() const { return mOffset; }
		[[nodiscard]] auto getUsage() const { return mUsage; }
		[[nodiscard]] auto getProperties() const { return mProperties; }

		VkDeviceMemory getMemory() const { return mImageMemory; }

		/// @brief Transfer Image Layout from old layout to new layout and insert necessary pipeline barrier.
		/// @param newLayout new layout to transfer to.
		/// @param srcStageMask source stage that needs to be finished before layout transition (producer).
		/// @param dstStageMask destination stage that needs to wait for the layout transition (consumer).
		/// @param subresourceRange the subresource range of the image to transition.
		/// @param commandPool command pool to allocate command buffer for the operation.
		/// @param commandBuffer optional command buffer to record the operation, if nullptr, a temporary command buffer will be created and submitted internally.
		void setImageLayout(
			VkImageLayout newLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange,
			const CommandPool::Ptr& commandPool,
			const CommandBuffer::Ptr& commandBUffer = nullptr);

		void fillImageData(size_t size, const void* pData, const CommandPool::Ptr& commandPool,const bool& isCubeMap = false);
		void CopyImageToCubeMap(const CommandPool::Ptr& commandPool, const VkImage& inSrcImage, VkImage inDstCubeMap, size_t inWidth, size_t inHeight, int inFace, int inMipmapLevel);
	private:
		uint32_t findMemoryType(Device::Ptr device, uint32_t typeFilter, VkMemoryPropertyFlags properties);



	private:

		Device::Ptr mDevice{ nullptr };
		VkDeviceMemory mImageMemory{ VK_NULL_HANDLE };
		VkImage mImage{ VK_NULL_HANDLE };
		VkImageView mImageView{ VK_NULL_HANDLE };
		VkImageLayout mImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		VkFormat mFormat{ VK_FORMAT_UNDEFINED };
		VkExtent3D mExtent{};
		VkDeviceSize mSize{ 0 };
		VkDeviceSize mAlignment{ 0 };
		VkDeviceSize mOffset{ 0 };

		VkImageUsageFlags mUsage{ 0 };

		VkMemoryPropertyFlags mProperties{ 0 };


	};
}