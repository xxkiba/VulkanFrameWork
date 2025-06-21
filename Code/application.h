#pragma once
#include "base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/window.h"
#include "vulkanWrapper/windowSurface.h"
#include "vulkanWrapper/swapChain.h"
#include "vulkanWrapper/shader.h"
#include "vulkanWrapper/pipeline.h"
#include "vulkanWrapper/renderPass.h"
#include "vulkanWrapper/commandPool.h"
#include "vulkanWrapper/commandBuffer.h"
#include "vulkanWrapper/semaphore.h"
#include "vulkanWrapper/fence.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/image.h"
#include "vulkanWrapper/sampler.h"
#include "vulkanWrapper/constantRange.h"

#include "offscreenRender/offscreenRenderTarget.h"
#include "offscreenRender/OffscreenSceneNode.h"
#include "offscreenRender/offscreenPipeline.h"
#include "texture/HDRI.h"

#include "texture/texture.h"
#include "uniformManager.h"
#include "pushConstantManager.h"
#include "Camera.h"
#include "SceneNode.h"
#include "model.h"
namespace FF {



	class Application:public std::enable_shared_from_this<Application> {
	public:
		Application() = default;
		~Application() = default;

		void run();

		void onMouseMove(double xpos, double ypos);

		void onKeyPress(CAMERA_MOVE moveDirection);
		float GetFrameTime();

	private:
		void initWindow();

		void initScene();

		void initVulkan();

		void mainLoop();

		void render();

		void cleanUp();

	private:
		Wrapper::Pipeline::Ptr createPipeline(const std::string& vertexShaderFile, const std::string& fragShaderFile);
		Wrapper::Pipeline::Ptr createScreenQuadPipeline(Wrapper::RenderPass::Ptr inRenderpass);
		Wrapper::RenderPass::Ptr createRenderPassForSwapChain();
		void createRenderPass();
		void createCommandBuffers();
		void createSyncObjects();
		void createUniformParameters();
		//void createTexture();

		//Recreate the swap chain: When the window is resized, the swap chain must be recreated, Frame View Pipeline RenderPass all need to be recreated
		void cleanUpSwapChain();
		void cleanUpOffScreenResources();
		void recreateSwapChain();

	private:
		int mWidth{ 1280 };
		int mHeight{ 720 };


	private:
		int mCurrentFrame{ 0 };
		Wrapper::Instance::Ptr mInstance{ nullptr };
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::Window::Ptr mWindow{ nullptr };
		Wrapper::WindowSurface::Ptr mSurface{ nullptr };
		Wrapper::SwapChain::Ptr mSwapChain{ nullptr };

		Wrapper::Pipeline::Ptr mPipeline{ nullptr };
		Wrapper::Pipeline::Ptr mScreenQuadPipeline{ nullptr }; // For rendering the offscreen render target to the screen
		Wrapper::Pipeline::Ptr mBattleFirePipeline{ nullptr };

		Wrapper::RenderPass::Ptr mRenderPass{ nullptr };
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };
		

		std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};
		std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
		std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
		std::vector<Wrapper::Fence::Ptr> mFences{};

		UniformManager::Ptr mUniformManager{ nullptr };
		PushConstantManager::Ptr mPushConstantManager{ nullptr };

		//Image and textures
		//Texture::Ptr mTexture{ nullptr }; 

		Model::Ptr mModel{ nullptr };
		NVPMatrices mNVPMatrices{};
		cameraParameters mCameraParameters{};
		SceneNode::Ptr mSphereNode{ nullptr };
		OffscreenSceneNode::Ptr mOffscreenSphereNode{ nullptr };
		OffscreenSceneNode::Ptr mSkyBoxNode{ nullptr };

		OffscreenPipeline::Ptr mSkyBoxPipeline{ nullptr };

		OffscreenRenderTarget::Ptr mOffscreenRenderTarget{ nullptr };

		bool useBattleFirePipeline{ true };
		//Camera mCamera{};
	};
}