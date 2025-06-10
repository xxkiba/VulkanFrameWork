#pragma once

#include "../base.h"
#include "../vulkanWrapper/image.h"
#include "../vulkanWrapper/sampler.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/commandPool.h"

namespace FF {
	class Texture {
	public:
		using Ptr = std::shared_ptr<Texture>;
		static Ptr create(const Wrapper::Device::Ptr& device,const Wrapper::CommandPool::Ptr &commandPool, const std::string& filePath) {
			return std::make_shared<Texture>(device, commandPool,filePath);
		}
		static Ptr create(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::array<std::string, 6>& cubemapPaths) {
			return std::make_shared<Texture>(device, commandPool,cubemapPaths);
		}
		Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr &commandPool,const std::string& filePath);
		Texture(const Wrapper::Device::Ptr& device,
			const Wrapper::CommandPool::Ptr& commandPool,
			const std::array<std::string, 6>& cubemapPaths);
		~Texture();
		void createTextureImageView();
		void createTextureSampler();
		void destroyTextureImageView();
		void destroyTextureSampler();
		void updateTextureImage(const FF::Wrapper::CommandPool::Ptr& commandPool);
		[[nodiscard]] const FF::Wrapper::Image::Ptr& getImage() const { return mImage; }
		[[nodiscard]] const FF::Wrapper::Sampler::Ptr& getSampler() const { return mSampler; }
		[[nodiscard]]  VkDescriptorImageInfo& getImageInfo()  { return mImageInfo; }
		[[nodiscard]] const std::string& getFilePath() const { return mFilePath; }

	private:
		Wrapper::Image::Ptr mImage{ nullptr };
		Wrapper::Sampler::Ptr mSampler{ nullptr };
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
		VkDescriptorImageInfo mImageInfo{};
		std::string mFilePath;

	};
}