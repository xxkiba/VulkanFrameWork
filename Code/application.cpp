#include "application.h"
#include "vulkanWrapper/image.h"


namespace FF {

	void Application::run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::onMouseMove(double xpos, double ypos) {
		mSphereNode->mCamera.onMouseMove(xpos, ypos);
	}

	void Application::onKeyPress(CAMERA_MOVE moveDirection) {
		mSphereNode->mCamera.move(moveDirection);
	}

	void Application::initWindow() {
		
		mWindow = Wrapper::Window::create(mWidth, mHeight);
		mWindow->setApplication(shared_from_this());

		mSphereNode = SceneNode::create();

		mSphereNode->mCamera.lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//mSphereNode->mCamera.update();

		mSphereNode->mCamera.setPerpective(60.0f, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 100.0f);

		mSphereNode->mCamera.setSpeed(0.001f);
	}

	void Application::initVulkan() {
		mInstance = Wrapper::Instance::create(true);
		mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);
		mDevice = Wrapper::Device::create(mInstance,mSurface);

		mCommandPool = Wrapper::CommandPool::create(mDevice);


		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		//mWidth = mSwapChain->getSwapChainExtent().width;
		//mHeight = mSwapChain->getSwapChainExtent().height;
		
		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		
		mSphereNode->mUniformManager = UniformManager::create();
		mSphereNode->mUniformManager->init(mDevice,mCommandPool, mSwapChain->getImageCount());

		mSphereNode->mMaterial = Material::create();
		std::vector<std::string> textureFiles;
		textureFiles.push_back("assets/book.jpg");
		textureFiles.push_back("assets/diffuse.jpg");
		textureFiles.push_back("assets/metal.jpg");
		mSphereNode->mMaterial->init(mDevice, mCommandPool, textureFiles,mSwapChain->getImageCount());

		mPushConstantManager = PushConstantManager::create();
		mPushConstantManager->init();

		// Create a model
		Model::Ptr commonModel = Model::create(mDevice);
		if (useBattleFirePipeline) {
			commonModel->loadBattleFireModel("assets/Sphere.rhsm", mDevice);
			mSphereNode->mModels.push_back(commonModel);
			mBattleFirePipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
			mSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));
			mPipeline = createPipeline("shaders/testVs.spv", "shaders/testFs.spv");
		}
		else {
			commonModel->loadModel("assets/book.obj", mDevice);
			mSphereNode->mModels.push_back(commonModel);
			mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
			mSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));
			mPipeline = createPipeline("shaders/vs.spv","shaders/fs.spv");
		}


		createCommandBuffers();

		createSyncObjects();

		//createTexture();
		


	}

	// Create a pipeline
	Wrapper::Pipeline::Ptr  Application::createPipeline(const std::string& vertexShaderFile,const std::string& fragShaderFile) {
		// Create a pipeline using the shader
		// mPipeline = Wrapper::Pipeline::create(mDevice, mSwapChain, mShader);

		Wrapper::Pipeline::Ptr mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);

		// Set up viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = static_cast<float>(mHeight);
		viewport.width = static_cast<float>(mWidth);
		viewport.height = -static_cast<float>(mHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight) };

		mPipeline->setViewports({ viewport });
		mPipeline->setScissors({ scissor });

		std::vector<Wrapper::Shader::Ptr> shaderGroup;
		auto vertexShader = Wrapper::Shader::create(mDevice, vertexShaderFile, VK_SHADER_STAGE_VERTEX_BIT, "main");
		//auto vertexShader = Wrapper::Shader::create(mDevice, "shaders/testVs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(vertexShader);
		auto fragmentShader = Wrapper::Shader::create(mDevice, fragShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		//auto fragmentShader = Wrapper::Shader::create(mDevice, "shaders/testFs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(fragmentShader);

		mPipeline->setShaderGroup(shaderGroup);

		// Set viewport and scissor
		mPipeline->mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mPipeline->mViewportState.viewportCount = static_cast<uint32_t>(mPipeline->mViewports.size());
		mPipeline->mViewportState.pViewports = mPipeline->mViewports.data();
		// Set scissor
		mPipeline->mViewportState.scissorCount = static_cast<uint32_t>(mPipeline->mScissors.size());
		mPipeline->mViewportState.pScissors = mPipeline->mScissors.data();

		//Dynamic state if needed
		mPipeline->mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		mPipeline->mDynamicState.dynamicStateCount = 0; // No dynamic state for now
		mPipeline->mDynamicState.pDynamicStates = nullptr; // No dynamic state for now

		// Layout of vertex data
		auto bindingDescriptions = mSphereNode->mModels[0]->getVertexInputBindingDescriptions();
		auto attributeDescriptions = mSphereNode->mModels[0]->getAttributeDescriptions();

		// Vertex input state
		mPipeline->mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mPipeline->mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		mPipeline->mVertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
		mPipeline->mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		mPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

		// Input assembly state
		mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		// Rasterization state
		mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;	// Need GPU feature for other modes
		mPipeline->mRasterState.lineWidth = 1.0f;					// Need GPU feature for values greater than 1.0f
		mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
		mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		mPipeline->mRasterState.rasterizerDiscardEnable = VK_FALSE;	// sream out, transform feedback, etc.
		mPipeline->mRasterState.depthClampEnable = VK_FALSE;	// Need GPU feature for this

		mPipeline->mRasterState.depthBiasEnable = VK_FALSE;
		mPipeline->mRasterState.depthBiasConstantFactor = 0.0f;
		mPipeline->mRasterState.depthBiasClamp = 0.0f;
		mPipeline->mRasterState.depthBiasSlopeFactor = 0.0f;// Slope factor for depth bias, used for shadow mapping and other effects

		// Multisample state 
		mPipeline->mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mPipeline->mMultisampleState.sampleShadingEnable = VK_FALSE;	// Need GPU feature for this
		mPipeline->mMultisampleState.rasterizationSamples = mDevice->getMaxUsableSampleCount();	// Use the maximum sample count supported by the device
		mPipeline->mMultisampleState.minSampleShading = 1.0f;
		mPipeline->mMultisampleState.pSampleMask = nullptr;
		mPipeline->mMultisampleState.alphaToCoverageEnable = VK_FALSE;
		mPipeline->mMultisampleState.alphaToOneEnable = VK_FALSE;

		//depth stencil state
		mPipeline->mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mPipeline->mDepthStencilState.depthTestEnable = VK_TRUE;	// Enable depth test
		mPipeline->mDepthStencilState.depthWriteEnable = VK_TRUE;	// Enable depth write
		mPipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;	// Depth compare operation
		mPipeline->mDepthStencilState.depthBoundsTestEnable = VK_FALSE;	// Depth bounds test, used for shadow mapping and other effects
		mPipeline->mDepthStencilState.stencilTestEnable = VK_FALSE;	// Stencil test, used for shadow mapping and other effects
		mPipeline->mDepthStencilState.front = {};	// Front stencil state, used for shadow mapping and other effects
		mPipeline->mDepthStencilState.back = {};	// Back stencil state, used for shadow mapping and other effects
		mPipeline->mDepthStencilState.minDepthBounds = 0.0f;	// Minimum depth bounds, used for shadow mapping and other effects
		mPipeline->mDepthStencilState.maxDepthBounds = 1.0f;	// Maximum depth bounds, used for shadow mapping and other effects


		// Color blend state
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		mPipeline->pushBlendAttachment(blendAttachment);

		// Blend has two way to calculate, the first one is to use the color and alpha blend factor, the second one is to use bitwise operation
		// If logicOp is set to VK_LOGIC_OP_COPY, the color and alpha blend factor will be ignored
		// ColorWrite Masks are still valid even if blendEnable is set to VK_FALSE
		// We might have more than one FrameBuffer, so we need to set the attachmentCount
		mPipeline->mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mPipeline->mBlendState.logicOpEnable = VK_FALSE;
		mPipeline->mBlendState.logicOp = VK_LOGIC_OP_COPY;
		// Blending
		mPipeline->mBlendState.attachmentCount = static_cast<uint32_t>(mPipeline->mBlendAttachmentStates.size());
		mPipeline->mBlendState.pAttachments = mPipeline->mBlendAttachmentStates.data();
		
		// Set the blend constants
		mPipeline->mBlendState.blendConstants[0] = 0.0f;
		mPipeline->mBlendState.blendConstants[1] = 0.0f;
		mPipeline->mBlendState.blendConstants[2] = 0.0f;
		mPipeline->mBlendState.blendConstants[3] = 0.0f;

		// Uniform transfer
		mPipeline->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		
		auto layout0 = mSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mSphereNode->mMaterial->getDescriptorLayout()->getLayout();

		std::vector<VkDescriptorSetLayout> layouts = { layout0, layout1 };
		mPipeline->mPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		mPipeline->mPipelineLayoutInfo.pSetLayouts = layouts.data();
		auto pushConstantRange = mPushConstantManager->getPushConstantRanges()->getPushConstantRange();
		// Transform the push constant ranges to VkPushConstantRange
		mPipeline->mPipelineLayoutInfo.pushConstantRangeCount = 1;
		mPipeline->mPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		mPipeline->build();

		return mPipeline;
	}



	void Application::cleanUpSwapChain() {
		// Release GPU resources
		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mFences.clear();

		// Release command buffers
		mCommandBuffers.clear();

		mPipeline.reset();
		mBattleFirePipeline.reset();
		mRenderPass.reset();
		mSwapChain.reset();

	}

	void Application::createUniformParameters() {


	}


	void Application::recreateSwapChain() {

		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		}

		vkDeviceWaitIdle(mDevice->getDevice());

		cleanUpSwapChain();

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getSwapChainExtent().width;
		mHeight = mSwapChain->getSwapChainExtent().height;

		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);


		createCommandBuffers();

		createSyncObjects();
	}

	void Application::createRenderPass() {
		// Create a render pass
		// 
		// 0: Final output color attachment
		// 1: Resolve image(MultiSample)
		// 2: Depth attachment
		
		// 0: Final output color attachment, created by the swap chain, target of resolve operation, is also the target to be set in subpass
		// Description of input canvas
		VkAttachmentDescription finalAttachment{};
		finalAttachment.format = mSwapChain->getSwapChainImageFormat();
		finalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		finalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		finalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		finalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		finalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		finalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		finalAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		mRenderPass->addAttachment(finalAttachment);

		// Description of indexes and format of the attachments
		VkAttachmentReference finalAttachmentRef{};
		finalAttachmentRef.attachment = 0;
		finalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// 1: Resolve image(MultiSample), source of multisample operation
		VkAttachmentDescription multiAttachment{};
		multiAttachment.format = mSwapChain->getSwapChainImageFormat();
		multiAttachment.samples = mDevice->getMaxUsableSampleCount();
		multiAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		multiAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		multiAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		multiAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		multiAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		multiAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // The final layout of the multisample image is color attachment optimal
		mRenderPass->addAttachment(multiAttachment);

		VkAttachmentReference multiAttachmentRef{};
		multiAttachmentRef.attachment = 1; // The multisample image is the second attachment
		multiAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // The layout of the multisample image is color attachment optimal


		// 3: Depth attachment
		// Description of depth canvas
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = Wrapper::Image::findDepthFormat(mDevice);
		depthAttachment.samples = mDevice->getMaxUsableSampleCount();
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		mRenderPass->addAttachment(depthAttachment);

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 2;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



		// Create subpass
		Wrapper::SubPass subpass{};
		subpass.addColorAttachmentReference(multiAttachmentRef);
		subpass.setDepthStencilAttachmentReference(depthAttachmentRef);
		subpass.setResolveAttachmentReference(finalAttachmentRef);
		subpass.buildSubPassDescription();

		mRenderPass->addSubpass(subpass);

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		mRenderPass->addDependency(dependency);
		mRenderPass->buildRenderPass();
	}

	void Application::mainLoop() {
		while (!mWindow->shouldClose()) {
			mWindow->pollEvents();
			mWindow->processEvents();

			//mModel->update();

			mNVPMatrices.mViewMatrix = mSphereNode->mCamera.getViewMatrix();
			mNVPMatrices.mProjectionMatrix = mSphereNode->mCamera.getProjectMatrix();
			mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mNVPMatrices.mViewMatrix));

			mSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mSphereNode->mModels[0]->getUniform(), mCurrentFrame);

			render();
		}

		vkDeviceWaitIdle(mDevice->getDevice());
	}

	void Application::createCommandBuffers() {
		// Create command buffers
		mCommandBuffers.resize(mSwapChain->getImageCount());
		for (size_t i = 0; i < mSwapChain->getImageCount(); i++) {
			mCommandBuffers[i] = Wrapper::CommandBuffer::create(mDevice, mCommandPool);
		}
		for (size_t i = 0; i < mSwapChain->getImageCount(); i++) {
			

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mRenderPass->getRenderPass();
			renderPassInfo.framebuffer = mSwapChain->getSwapChainFramebuffers()[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mSwapChain->getSwapChainExtent();

			std::vector<VkClearValue> clearValues;


			//0: final output color attachment 1:multisample image 2: depth attachment
			VkClearValue clearFinalColor{};
			clearFinalColor.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			clearValues.push_back(clearFinalColor);

			//1: Multisample image
			VkClearValue clearMultiSample{};
			clearMultiSample.color = { 0.1f, 0.4f, 0.6f, 1.0f };
			clearValues.push_back(clearMultiSample);

			//2: Depth attachment
			VkClearValue clearDepth{};
			clearDepth.depthStencil = { 1.0f, 0 };
			clearValues.push_back(clearDepth);

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			mCommandBuffers[i]->beginCommandBuffer();
			// Begin render pass
			mCommandBuffers[i]->beginRenderPass(renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


			mCommandBuffers[i]->bindGraphicPipeline(mPipeline);

			std::vector<VkDescriptorSet> descriptorSets = { mSphereNode->mUniformManager->getDescriptorSet(mCurrentFrame) , mSphereNode->mMaterial->getDescriptorSet(mCurrentFrame) };
			mCommandBuffers[i]->bindDescriptorSets(mPipeline->getPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data());


			mCommandBuffers[i]->pushConstants(mPipeline->getPipelineLayout(), mPushConstantManager->getConstantParam().stageFlags,
			mPushConstantManager->getConstantParam().offset, mPushConstantManager->getConstantParam().size, &mPushConstantManager->getConstantData());



			//mModel->draw(mCommandBuffers[i]);
			mSphereNode->draw(mCommandBuffers[i]);

			mCommandBuffers[i]->endRenderPass();
			mCommandBuffers[i]->endCommandBuffer();
		}
	}

	void Application::createSyncObjects() {
		for (int i = 0; i < mSwapChain->getImageCount(); i++) {
			mImageAvailableSemaphores.push_back(Wrapper::Semaphore::create(mDevice));
			mRenderFinishedSemaphores.push_back(Wrapper::Semaphore::create(mDevice));
			mFences.push_back(Wrapper::Fence::create(mDevice, true));
		}
	}
	
	//void Application::createTexture() {
	//	// Create a texture
	//	std::string texturePath = "assets/dragonball.jpg";
	//	mTexture = Texture::create(mDevice, mCommandPool, texturePath);

	//}

	void Application::render() {

		// Wait for the fence to be signaled
		mFences[mCurrentFrame]->waitForFence();

		// Acquire the next image from the swap chain
		uint32_t imageIndex = 0;

		VkResult result = vkAcquireNextImageKHR(
			mDevice->getDevice(),
			mSwapChain->getSwapChain(),
			std::numeric_limits<uint64_t>::max(),
			mImageAvailableSemaphores[mCurrentFrame]->getSemaphore(),
			VK_NULL_HANDLE,
			&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Error: failed to acquire swap chain image!");
		}



		// Submit the command buffer to the queue
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		//Synchronize the image with the semaphore
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame]->getSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };


		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Designate the command buffer to be submitted
		submitInfo.commandBufferCount = 1;

		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex]->getCommandBuffer();

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		mFences[mCurrentFrame]->resetFence();
		if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mFences[mCurrentFrame]->getFence()) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to submit draw command buffer!");
		}

		// Present the image to the swap chain
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);
		//presentInfo.pResults = nullptr;
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow->mWindowResized) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
		}else if (result != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to present swap chain image!");
		}
		mCurrentFrame = (mCurrentFrame + 1) % mSwapChain->getImageCount();
	}

	void Application::cleanUp() {
		vkDeviceWaitIdle(mDevice->getDevice());
		if (mPipeline) {
			mPipeline.reset();
		}
		if (mRenderPass) {
			mRenderPass.reset();
		}
		if (mSwapChain) {
			mSwapChain.reset();
		}
		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
		mWindow.reset();
	}
}