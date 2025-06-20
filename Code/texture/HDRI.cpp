#include "HDRI.h"

namespace FF {


	HDRI::HDRI(Wrapper::Device::Ptr device,Wrapper::CommandPool::Ptr commandPool)
		: mDevice(device),mCommandPool(commandPool) {
		// Initialize the offscreen render target and pipeline if needed
		// mOffscreenRenderTarget = Wrapper::OffscreenRenderTarget::create(mDevice);
		// mOffscreenPipeline = Wrapper::OffscreenPipeline::create(mDevice);
	}

	HDRI::~HDRI() {
		// Cleanup resources if needed
		mOffscreenRenderTarget.reset();
		mOffscreenPipeline.reset();
		mOffscreenSphereNode.reset();
		mImage.reset();
		mSampler.reset();
	}

	void HDRI::InitMatrices() {
		//gCaptureCameras[0].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		//gCaptureCameras[1].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		//gCaptureCameras[2].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		//gCaptureCameras[3].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//gCaptureCameras[4].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		//gCaptureCameras[5].lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

		gCaptureCameras[0].m_vMatrix = lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		gCaptureCameras[1].m_vMatrix = lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		gCaptureCameras[2].m_vMatrix = lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		gCaptureCameras[3].m_vMatrix = lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		gCaptureCameras[4].m_vMatrix = lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		gCaptureCameras[5].m_vMatrix = lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

		gCaptureCameras[0].setPerpective(90.0f, 1.0f, 0.1f, 100.0f);
		gCaptureCameras[1].setPerpective(90.0f, 1.0f, 0.1f, 100.0f);
		gCaptureCameras[2].setPerpective(90.0f, 1.0f, 0.1f, 100.0f);
		gCaptureCameras[3].setPerpective(90.0f, 1.0f, 0.1f, 100.0f);
		gCaptureCameras[4].setPerpective(90.0f, 1.0f, 0.1f, 100.0f);
		gCaptureCameras[5].setPerpective(90.0f, 1.0f, 0.1f, 100.0f);
	}
	void HDRI::HDRI2CubeMap(
		const std::string& filePath,
		Wrapper::Image::Ptr& cubMapImage,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath, std::string inFragShaderPath) {

		InitMatrices();
		Texture::Ptr hdriTexture = Texture::createHDRITexture(mDevice, mCommandPool, filePath);



		mOffscreenRenderTarget = OffscreenRenderTarget::create(
			mDevice, mCommandPool,
			texWidth, texHeight,
			1,
			VK_FORMAT_R32G32B32A32_SFLOAT, // Color format
			VK_FORMAT_D24_UNORM_S8_UINT, // Depth format
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL // Final layout for the offscreen render target, copy from render target to cubemap image, need to be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		);

		Model::Ptr skyboxModel = Model::create(mDevice);
		skyboxModel->loadBattleFireComponent("assets/skybox.staticmesh", mDevice);
		mOffscreenSphereNode = OffscreenSceneNode::create();
		mOffscreenSphereNode->mUniformManager = UniformManager::create();
		mOffscreenSphereNode->mUniformManager->init(mDevice, mCommandPool, 1);
		mOffscreenSphereNode->mUniformManager->build();
		mOffscreenSphereNode->mModels.push_back(skyboxModel);
		mOffscreenSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));


		std::vector<std::string> textureFiles;
		textureFiles.push_back("assets/book.jpg");
		textureFiles.push_back("assets/diffuse.jpg");
		textureFiles.push_back("assets/metal.jpg");

		mOffscreenSphereNode->mMaterial = Material::create();
		mOffscreenSphereNode->mMaterial->attachTexturePaths(textureFiles);
		mOffscreenSphereNode->mMaterial->attachImages({ hdriTexture ->getImage()});
		mOffscreenSphereNode->mMaterial->init(mDevice, mCommandPool, 1);

		auto layout0 = mOffscreenSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mOffscreenSphereNode->mMaterial->getDescriptorLayout()->getLayout();

		mOffscreenPipeline = OffscreenPipeline::create(mDevice);
		mOffscreenPipeline->build(
			mOffscreenRenderTarget->getRenderPass(),
			texWidth, texHeight,
			inVertShaderPath, inFragShaderPath,
			{ layout0,layout1 },
			mOffscreenSphereNode->mModels[0]->getVertexInputBindingDescriptions(),
			mOffscreenSphereNode->mModels[0]->getAttributeDescriptions(),
			nullptr, // No push constants
			mDevice->getMaxUsableSampleCount(),
			VK_FRONT_FACE_CLOCKWISE);


		VkRenderPassBeginInfo offScreenRenderPassBeginInfo{};
		offScreenRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		offScreenRenderPassBeginInfo.renderPass = mOffscreenRenderTarget->getRenderPass()->getRenderPass();
		offScreenRenderPassBeginInfo.framebuffer = mOffscreenRenderTarget->getOffScreenFramebuffers()[0];
		offScreenRenderPassBeginInfo.renderArea.offset = { 0, 0 };
		offScreenRenderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };
		std::vector<VkClearValue> cvs;
		//0: final output color attachment 1:multisample image 2: depth attachment
		VkClearValue offScreenClearFinalColor{};
		offScreenClearFinalColor.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearFinalColor);

		//1: Multisample image
		VkClearValue offScreenClearMultiSample{};
		offScreenClearMultiSample.color = { 0.0f, 0.0f, 0.0f, 0.0f };
		cvs.push_back(offScreenClearMultiSample);

		//2: Depth attachment
		VkClearValue offScreenClearDepth{};
		offScreenClearDepth.depthStencil = { 1.0f, 0 };
		cvs.push_back(offScreenClearDepth);

		offScreenRenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(cvs.size());
		offScreenRenderPassBeginInfo.pClearValues = cvs.data();

		for (int i = 0; i < 6; i++) {

			Wrapper::CommandBuffer::Ptr mCommandBuffer = Wrapper::CommandBuffer::create(mDevice, mCommandPool);


			mNVPMatrices.mViewMatrix = gCaptureCameras[i].getViewMatrix();
			mNVPMatrices.mProjectionMatrix = gCaptureCameras[i].getProjectMatrix();
			mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mNVPMatrices.mViewMatrix));
			mCameraParameters.CameraWorldPosition = gCaptureCameras[i].getCamPosition();

			mOffscreenSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mOffscreenSphereNode->mModels[0]->getUniform(), mCameraParameters, 0);


			mCommandBuffer->beginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

			// Begin offscreen render pass
			mCommandBuffer->beginRenderPass(offScreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			mCommandBuffer->bindGraphicPipeline(mOffscreenPipeline->getPipeline());
			std::vector<VkDescriptorSet> offscreenDescriptorSets = { mOffscreenSphereNode->mUniformManager->getDescriptorSet(0) , mOffscreenSphereNode->mMaterial->getDescriptorSet(0) };
			mCommandBuffer->bindDescriptorSets(mOffscreenPipeline->getPipeline()->getPipelineLayout(), 0, offscreenDescriptorSets.size(), offscreenDescriptorSets.data());

			mOffscreenSphereNode->draw(mCommandBuffer);
			mCommandBuffer->endRenderPass();

			// Copy the rendered image to the cubemap image
			mCommandBuffer->CopyRTImageToCubeMap(
				mOffscreenRenderTarget->getRenderTargetImages()[0]->getImage(),
				cubMapImage->getImage(),
				texWidth, texHeight, i, 0);


			mCommandBuffer->endCommandBuffer();
			// Submit the command buffer
			mCommandBuffer->submitCommandBuffer(mDevice->getGraphicQueue());
			// Wait for the command buffer to finish
			mCommandBuffer->waitCommandBuffer(mDevice->getGraphicQueue());

		}

	}

	Wrapper::Image::Ptr HDRI::LoadHDRICubeMapFromFile(
		const Wrapper::Device::Ptr& device,
		const Wrapper::CommandPool::Ptr& commandPool,
		const std::string& filePath,
		uint32_t texWidth, uint32_t texHeight,
		std::string inVertShaderPath, std::string inFragShaderPath) {
        // Create the cubemap image
        Wrapper::Image::Ptr mImage = Wrapper::Image::create(
            mDevice, texWidth, texHeight,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, true);

		// Set the image layout for transfer
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 6; // Cubemap has 6 faces

		mImage->setImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			subresourceRange,
			mCommandPool);

		InitMatrices();
		// Load the HDR image data
		HDRI2CubeMap(filePath, mImage, texWidth, texHeight, inVertShaderPath, inFragShaderPath);

		mImage->setImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			subresourceRange,
			mCommandPool);

		return mImage;
	}
}