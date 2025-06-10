#pragma once

#include "../base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "renderPass.h"
#include "image.h"
#include "commandPool.h"

namespace FF::Wrapper {

	struct SwapChainSupportInfo {
		VkSurfaceCapabilitiesKHR mCapabilities{};
		std::vector<VkSurfaceFormatKHR> mFormats;
		std::vector<VkPresentModeKHR> mPresentModes;
	};

	class SwapChain {
	public:
		using Ptr = std::shared_ptr<SwapChain>;
		static Ptr create(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface, const CommandPool::Ptr &commandPool) { return std::make_shared<SwapChain>(device, window, surface, commandPool); }
		SwapChain(const Device::Ptr &device,const Window::Ptr &window, const WindowSurface::Ptr &surface, const CommandPool::Ptr& commandPool);
		~SwapChain();

		SwapChainSupportInfo querySwapChainSupportInfo();

		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void createFrameBuffers(const RenderPass::Ptr& renderPass);

	public:

		[[nodiscard]] auto getSwapChainImageFormat() const { return mSwapChainFormat; }
		[[nodiscard]] auto getSwapChainExtent() const { return mSwapChainExtent; }
		[[nodiscard]] auto getSwapChainImages() const { return mSwapChainImages; }
		[[nodiscard]] auto getSwapChainImageViews() const { return mSwapChainImageViews; }
		[[nodiscard]] auto getSwapChain() const { return mSwapChain; }
		[[nodiscard]] auto getSwapChainFramebuffers() const { return mSwapChainFramebuffers; }
		[[nodiscard]] auto getImageCount() const { return mImageCount; }

		uint32_t acquireNextImage(VkSemaphore semaphore, VkFence fence = VK_NULL_HANDLE) {
			uint32_t imageIndex;
			vkAcquireNextImageKHR(mDevice->getDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(), semaphore, fence, &imageIndex);
			return imageIndex;
		}

	private:
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1);
	private:
		Device::Ptr mDevice{ nullptr };
		Window::Ptr mWindow{ nullptr };
		WindowSurface::Ptr mSurface{ nullptr };
		CommandPool::Ptr mCommandPool{ nullptr };

		VkFormat mSwapChainFormat;
		VkExtent2D mSwapChainExtent;
		uint32_t mImageCount{ 0 };

		// Swap chain images and image views
		// The swapchain is incharge of creating and destroying the image 
		std::vector<VkImage> mSwapChainImages{};

		// Framework to control the swapchain images
		std::vector<VkImageView> mSwapChainImageViews{};

		// Depth image for the swapchain
		std::vector<Image::Ptr> mDepthImages{};

		// Multisampling image for the swapchain, transient image
		std::vector<Image::Ptr> mMultisampleImages{};


		std::vector<VkFramebuffer> mSwapChainFramebuffers{};

		VkSwapchainKHR mSwapChain{ VK_NULL_HANDLE };
	};
}