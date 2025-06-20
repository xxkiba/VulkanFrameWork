#pragma once
#include "../base.h"
#include "../vulkanWrapper/image.h"
#include "../vulkanWrapper/sampler.h"
#include "../vulkanWrapper/device.h"
#include "../offscreenRender/offscreenRenderTarget.h"
#include "../offscreenRender/offscreenPipeline.h"
#include "../offscreenRender/OffscreenSceneNode.h"
#include "../model.h"
#include "../Camera.h"
#include "texture.h"

namespace FF {
	class HDRI {
	public:
		using Ptr = std::shared_ptr<HDRI>;
		static Ptr create(Wrapper::Device::Ptr device, Wrapper::CommandPool::Ptr commandPool) {
			return std::make_shared<HDRI>(device,commandPool);
		}

		HDRI(Wrapper::Device::Ptr device, Wrapper::CommandPool::Ptr commandPool);
		~HDRI();

		Wrapper::Image::Ptr LoadHDRICubeMapFromFile(
			const Wrapper::Device::Ptr& device,
			const Wrapper::CommandPool::Ptr& commandPool,
			const std::string& filePath,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);

		void HDRI2CubeMap(
			const std::string& filePath,
			Wrapper::Image::Ptr& cubMapImage,
			uint32_t texWidth = 1024, uint32_t texHeight = 1024,
			std::string inVertShaderPath = "shaders/SkyboxVert.spv",
			std::string inFragShaderPath = "shaders/SkyBoxFrag.spv");
		void InitMatrices();

	private:
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::Image::Ptr mImage{ nullptr };
		Wrapper::Sampler::Ptr mSampler{ nullptr };
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };

		OffscreenRenderTarget::Ptr mOffscreenRenderTarget{ nullptr };
		OffscreenPipeline::Ptr mOffscreenPipeline{ nullptr };
		OffscreenSceneNode::Ptr mOffscreenSphereNode{ nullptr };

		VkDescriptorImageInfo mImageInfo{};
		std::string mFilePath;
		std::string mVertShaderPath;
		std::string mFragShaderPath;
		NVPMatrices mNVPMatrices{};
		cameraParameters mCameraParameters{};
		Camera gCaptureCameras[6];

	};

}
