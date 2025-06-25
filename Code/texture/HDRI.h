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
#include "../pushConstantManager.h"
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


		Wrapper::Image::Ptr generateDiffuseIrradianceMap(
			Wrapper::Image::Ptr hdriCubMapImage,
			const Wrapper::Device::Ptr& device,
			const Wrapper::CommandPool::Ptr& commandPool,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);

		void captureDiffuseIrradianceMap(
			Wrapper::Image::Ptr& hdriCubMapImage,
			Wrapper::Image::Ptr& diffuseIrradianceCubMapImage,
			uint32_t texWidth = 32, uint32_t texHeight =32,
			std::string inVertShaderPath = "shaders/SkyboxVert.spv",
			std::string inFragShaderPath = "shaders/CaptureDiffuseIrradianceFrag.spv");

		Wrapper::Image::Ptr generateSpecularPrefilterMap(
			Wrapper::Image::Ptr hdriCubMapImage,
			const Wrapper::Device::Ptr& device,
			const Wrapper::CommandPool::Ptr& commandPool,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);

		void captureSpecularPrefilterMap(
			Wrapper::Image::Ptr& hdriCubMapImage,
			Wrapper::Image::Ptr& specularPrefilterCubMapImage,
			uint32_t texWidth = 128, uint32_t texHeight = 128,
			std::string inVertShaderPath = "shaders/SkyboxVert.spv",
			std::string inFragShaderPath = "shaders/CaptureSpecularPrefilterFrag.spv");

		Wrapper::Image::Ptr generateBRDFLUT(
			const Wrapper::Device::Ptr& device,
			const Wrapper::CommandPool::Ptr& commandPool,
			uint32_t texWidth, uint32_t texHeight,
			std::string inVertShaderPath, std::string inFragShaderPath);



		void InitMatrices();

	private:
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::Image::Ptr mImage{ nullptr };
		Wrapper::Sampler::Ptr mSampler{ nullptr };
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };

		VkDescriptorImageInfo mImageInfo{};
		std::string mFilePath;
		std::string mVertShaderPath;
		std::string mFragShaderPath;
		NVPMatrices mNVPMatrices{};
		cameraParameters mCameraParameters{};
		Camera gCaptureCameras[6];

	};

}
