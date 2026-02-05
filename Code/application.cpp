#include "application.h"

namespace FF {

	void Application::run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::onMouseMove(double xpos, double ypos) {
		mSphereNode->mCamera.onMouseMove(xpos, ypos);
		mOffscreenSphereNode->mCamera.onMouseMove(xpos, ypos);
		mSkyBoxNode->mCamera.onMouseMove(xpos, ypos);
	}

	void Application::onKeyPress(CAMERA_MOVE moveDirection) {
		mSphereNode->mCamera.move(moveDirection);
		mOffscreenSphereNode->mCamera.move(moveDirection);
		mSkyBoxNode->mCamera.move(moveDirection);
	}

	float Application::GetFrameTime() {
		static double lastTime = 0.0;
		double currentTime = glfwGetTime();
		float deltaTime = static_cast<float>(currentTime - lastTime);
		lastTime = currentTime;
		return deltaTime;
	}
	void Application::initWindow() {
		
		mWindow = Wrapper::Window::create(mWidth, mHeight);
		mWindow->setApplication(shared_from_this());

		mSphereNode = SceneNode::create();
		mOffscreenSphereNode = OffscreenSceneNode::create();
		mSkyBoxNode = OffscreenSceneNode::create();

		//mSphereNode->mCamera.lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//mOffscreenSphereNode->mCamera.lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		
		mSphereNode->mCamera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, glm::vec3(1.0f, 1.0f, 1.0f));
		mOffscreenSphereNode->mCamera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, glm::vec3(0.0f, -0.2f, 1.0f));
		
		/* SkyBox Node Should be in the center */
		mSkyBoxNode->mCamera.Init(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, glm::vec3(0.0f, -0.2f, 1.0f));
		// 
		//mSphereNode->mCamera.update();

		mSphereNode->mCamera.setPerpective(45.0f, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 1000.0f);
		mOffscreenSphereNode->mCamera.setPerpective(45.0f, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 1000.0f);
		mSkyBoxNode->mCamera.setPerpective(45.0f, static_cast<float>(mWidth) / static_cast<float>(mHeight), 0.1f, 1000.0f);

		mSphereNode->mCamera.setSpeed(0.001f);
		mOffscreenSphereNode->mCamera.setSpeed(0.001f);
		mSkyBoxNode->mCamera.setSpeed(0.001f);
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
		mRenderPass = createRenderPassForSwapChain();
		//createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);
		mOffscreenRenderTarget = OffscreenRenderTarget::create(
			mDevice, mCommandPool,
			mWidth, mHeight,
			mSwapChain->getImageCount(),
			VK_FORMAT_R32G32B32A32_SFLOAT, // Color format
			VK_FORMAT_D24_UNORM_S8_UINT // Depth format
		);

		
		HDRI::Ptr hdri = HDRI::create(mDevice, mCommandPool);
		// HDRI cubemap
		Wrapper::Image::Ptr HDRICubemap = hdri->LoadHDRICubeMapFromFile(
			mDevice, mCommandPool,
			"assets/1.hdr",
			512, 512,
			"shaders/HDRI2CubemapVert.spv", "shaders/HDRI2CubemapFrag.spv"
		);
		// // Diffuse irradiance map
		Wrapper::Image::Ptr diffuseIrradianceMap = hdri->generateDiffuseIrradianceMap(
			HDRICubemap,
			mDevice, mCommandPool,
			32, 32,
			"shaders/SkyboxVert.spv", "shaders/CaptureDiffuseIrradianceFrag.spv"
		);
		 // Specular prefilter map
		Wrapper::Image::Ptr specularPrefilterMap = hdri->generateSpecularPrefilterMap(
			HDRICubemap,
			mDevice, mCommandPool,
			128, 128,
			"shaders/SkyboxVert.spv", "shaders/CaptureSpecularPrefilterFrag.spv"
		);
		// // BRDF LUT
		Wrapper::Image::Ptr brdfLUT = hdri->generateBRDFLUT(
			mDevice, mCommandPool,
			512, 512,
			"shaders/full_screen_triangle.spv", "shaders/generateBRDFFrag.spv"
		);


		mSphereNode->mUniformManager = UniformManager::create();
		mSphereNode->mUniformManager->init(mDevice,mCommandPool, mSwapChain->getImageCount());
		mSphereNode->mUniformManager->build();

		mSkyBoxNode->mUniformManager = UniformManager::create();
		mSkyBoxNode->mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());
		mSkyBoxNode->mUniformManager->attachCubeMap(HDRICubemap);
		mSkyBoxNode->mUniformManager->build();

		/*
		*	layout(set =0, binding = 4) uniform samplerCube U_prefilteredColor;
		*	layout(set = 0, binding = 5) uniform samplerCube U_DiffuseIrradiance;
		*	layout(set = 0, binding = 6) uniform sampler2D U_BRDFLUT;
		*/
		mOffscreenSphereNode->mUniformManager = UniformManager::create();
		mOffscreenSphereNode->mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());
		mOffscreenSphereNode->mUniformManager->attachCubeMap(specularPrefilterMap);
		mOffscreenSphereNode->mUniformManager->attachCubeMap(diffuseIrradianceMap);
		mOffscreenSphereNode->mUniformManager->attachImage(brdfLUT);

		//Helmet Images
		Wrapper::Image::Ptr Albedo = Wrapper::Image::createFromFile(mDevice, mCommandPool,"assets/DamagedHelmet/Albedo.jpg",VK_FORMAT_R8G8B8A8_UNORM);
		Wrapper::Image::Ptr Normal = Wrapper::Image::createFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Normal.jpg", VK_FORMAT_R8G8B8A8_UNORM);
		Wrapper::Image::Ptr Metallic = Wrapper::Image::createFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Metallic.png", VK_FORMAT_R8G8B8A8_UNORM);
		Wrapper::Image::Ptr Roughness = Wrapper::Image::createFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Roughness.png", VK_FORMAT_R8G8B8A8_UNORM);
		Wrapper::Image::Ptr AO = Wrapper::Image::createFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/AO.jpg", VK_FORMAT_R8G8B8A8_UNORM);
		Wrapper::Image::Ptr Emissive = Wrapper::Image::createFromFile(mDevice, mCommandPool, "assets/DamagedHelmet/Emissive.jpg", VK_FORMAT_R8G8B8A8_UNORM);

		//Helmet Images
		mOffscreenSphereNode->mUniformManager->attachMapImage(Albedo);
		mOffscreenSphereNode->mUniformManager->attachMapImage(Normal);
		mOffscreenSphereNode->mUniformManager->attachMapImage(Emissive);
		mOffscreenSphereNode->mUniformManager->attachMapImage(AO);
		mOffscreenSphereNode->mUniformManager->attachMapImage(Metallic);
		mOffscreenSphereNode->mUniformManager->attachMapImage(Roughness);

	

		mOffscreenSphereNode->mUniformManager->build();


		
		std::vector<std::string> textureFiles;
		textureFiles.push_back("assets/book.jpg");
		textureFiles.push_back("assets/diffuse.jpg");
		textureFiles.push_back("assets/metal.jpg");



		mOffscreenSphereNode->mMaterial = Material::create();
		mOffscreenSphereNode->mMaterial->attachTexturePaths(textureFiles);
		mOffscreenSphereNode->mMaterial->init(mDevice, mCommandPool, mSwapChain->getImageCount());

		mSphereNode->mMaterial = Material::create();
		//mSphereNode->mMaterial->attachTexturePaths(textureFiles);
		mSphereNode->mMaterial->attachImages(mOffscreenRenderTarget->getRenderTargetImages()); // Attach the offscreen render target images to the material
		mSphereNode->mMaterial->init(mDevice, mCommandPool,mSwapChain->getImageCount());

		//No material for the skybox, just use the cubemap texture



		mPushConstantManager = PushConstantManager::create();
		mPushConstantManager->init();

		// Create a model
		Model::Ptr commonModel = Model::create(mDevice);
		Model::Ptr offscreenModel = Model::create(mDevice);
		Model::Ptr skyboxModel = Model::create(mDevice);
		if (useBattleFirePipeline) {
			// Didn't really draw the sphere here, just need to load a model to make code work.
			commonModel->loadModel("assets/book.obj", mDevice);
			mSphereNode->mModels.push_back(commonModel);
			mSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));

			offscreenModel->loadBattleFireModel("assets/DamagedHelmet.staticmesh", mDevice);
			mOffscreenSphereNode->mModels.push_back(offscreenModel);
			mOffscreenSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));

			skyboxModel->loadBattleFireComponent("assets/skybox.staticmesh", mDevice);
			mSkyBoxNode->mModels.push_back(skyboxModel);
			mSkyBoxNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));

			mPipeline = createPipeline("shaders/pbr1Vert.spv", "shaders/pbr1Frag.spv");
		}
		else {
			commonModel->loadModel("assets/book.obj", mDevice);
			mSphereNode->mModels.push_back(commonModel);
			mSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));

			offscreenModel->loadBattleFireModel("assets/Sphere.rhsm", mDevice);
			mOffscreenSphereNode->mModels.push_back(offscreenModel);
			mOffscreenSphereNode->mModels[0]->setModelMatrix(glm::mat4(1.0f));

			mPipeline = createPipeline("shaders/vs.spv","shaders/fs.spv");
		}
		mScreenQuadPipeline = createScreenQuadPipeline(mRenderPass);
		mSkyBoxPipeline = OffscreenPipeline::create(mDevice);
		mSkyBoxPipeline->build(
			mOffscreenRenderTarget->getRenderPass(),
			mWidth, mHeight,
			"shaders/SkyboxVert.spv", "shaders/SkyBoxFrag.spv",
			{ mSkyBoxNode->mUniformManager->getDescriptorLayout()->getLayout() },
			mSkyBoxNode->mModels[0]->getVertexInputBindingDescriptions(),
			mSkyBoxNode->mModels[0]->getAttributeDescriptions(),
			nullptr, // No push constants
			mDevice->getMaxUsableSampleCount(),
			VK_FRONT_FACE_CLOCKWISE
		);

		createCommandBuffers();

		createSyncObjects();

		//createTexture();
		


	}

	// Create a pipeline
	Wrapper::Pipeline::Ptr  Application::createPipeline(const std::string& vertexShaderFile,const std::string& fragShaderFile) {
		// Create a pipeline using the shader
		// mPipeline = Wrapper::Pipeline::create(mDevice, mSwapChain, mShader);

		Wrapper::Pipeline::Ptr mPipeline = Wrapper::Pipeline::create(mDevice, mOffscreenRenderTarget->getRenderPass());

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
		shaderGroup.push_back(vertexShader);
		auto fragmentShader = Wrapper::Shader::create(mDevice, fragShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT, "main");
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
		auto bindingDescriptions = mOffscreenSphereNode->mModels[0]->getVertexInputBindingDescriptions();
		auto attributeDescriptions = mOffscreenSphereNode->mModels[0]->getAttributeDescriptions();

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
		
		auto layout0 = mOffscreenSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mOffscreenSphereNode->mMaterial->getDescriptorLayout()->getLayout();

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

	Wrapper::Pipeline::Ptr  Application::createScreenQuadPipeline(Wrapper::RenderPass::Ptr inRenderpass) {
		auto screenQuadPipeline = Wrapper::Pipeline::create(mDevice, inRenderpass);

		// Set up viewport and scissor
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mWidth);
		viewport.height = static_cast<float>(mHeight);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight) };

		screenQuadPipeline->setViewports({ viewport });
		screenQuadPipeline->setScissors({ scissor });


		// Set viewport and scissor
		screenQuadPipeline->mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		screenQuadPipeline->mViewportState.viewportCount = static_cast<uint32_t>(screenQuadPipeline->mViewports.size());
		screenQuadPipeline->mViewportState.pViewports = screenQuadPipeline->mViewports.data();
		// Set scissor
		screenQuadPipeline->mViewportState.scissorCount = static_cast<uint32_t>(screenQuadPipeline->mScissors.size());
		screenQuadPipeline->mViewportState.pScissors = screenQuadPipeline->mScissors.data();


		//Dynamic state if needed
		screenQuadPipeline->mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		screenQuadPipeline->mDynamicState.dynamicStateCount = 0; // No dynamic state for now
		screenQuadPipeline->mDynamicState.pDynamicStates = nullptr; // No dynamic state for now

		// shader
		auto vs = Wrapper::Shader::create(mDevice, "shaders/full_screen_triangle.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		auto fs = Wrapper::Shader::create(mDevice, "shaders/screen_quad.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		std::vector<Wrapper::Shader::Ptr> shaders = { vs, fs };
		screenQuadPipeline->setShaderGroup(shaders);

		// Full screen triangle, no need for vertex buffer
		screenQuadPipeline->mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		screenQuadPipeline->mVertexInputState.vertexBindingDescriptionCount = 0;
		screenQuadPipeline->mVertexInputState.pVertexBindingDescriptions = nullptr;
		screenQuadPipeline->mVertexInputState.vertexAttributeDescriptionCount = 0;
		screenQuadPipeline->mVertexInputState.pVertexAttributeDescriptions = nullptr;

		// Assembly
		screenQuadPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		screenQuadPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		screenQuadPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		// Raster/depth/multisample
		screenQuadPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		screenQuadPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;
		screenQuadPipeline->mRasterState.lineWidth = 1.0f;
		screenQuadPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT; 
		screenQuadPipeline->mRasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		screenQuadPipeline->mRasterState.rasterizerDiscardEnable = VK_FALSE;

		screenQuadPipeline->mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		screenQuadPipeline->mMultisampleState.sampleShadingEnable = VK_FALSE;
		screenQuadPipeline->mMultisampleState.rasterizationSamples = mDevice->getMaxUsableSampleCount();

		screenQuadPipeline->mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		screenQuadPipeline->mDepthStencilState.depthTestEnable = VK_FALSE;
		screenQuadPipeline->mDepthStencilState.depthWriteEnable = VK_FALSE;

		// Blend
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		screenQuadPipeline->pushBlendAttachment(blendAttachment);

		screenQuadPipeline->mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		screenQuadPipeline->mBlendState.attachmentCount = uint32_t(screenQuadPipeline->mBlendAttachmentStates.size());
		screenQuadPipeline->mBlendState.pAttachments = screenQuadPipeline->mBlendAttachmentStates.data();

		// Pipeline Layout
		screenQuadPipeline->mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		auto layout0 = mSphereNode->mUniformManager->getDescriptorLayout()->getLayout();
		auto layout1 = mSphereNode->mMaterial->getDescriptorLayout()->getLayout();

		std::vector<VkDescriptorSetLayout> layouts = { layout0, layout1 };
		screenQuadPipeline->mPipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		screenQuadPipeline->mPipelineLayoutInfo.pSetLayouts = layouts.data();
		auto pushConstantRange = mPushConstantManager->getPushConstantRanges()->getPushConstantRange();
		// Transform the push constant ranges to VkPushConstantRange
		screenQuadPipeline->mPipelineLayoutInfo.pushConstantRangeCount = 1;
		screenQuadPipeline->mPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		screenQuadPipeline->build();

		return screenQuadPipeline;
	}

	void Application::cleanUpSwapChain() {
		// Release GPU resources
		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mFences.clear();

		// Release command buffers
		mCommandBuffers.clear();

		mBattleFirePipeline.reset();
		mRenderPass.reset();
		mSwapChain.reset();
		mCurrentFrame = 0;

	}
	void Application::cleanUpOffScreenResources() {
		mOffscreenRenderTarget.reset();
		mScreenQuadPipeline.reset();
		mSkyBoxPipeline.reset();
		mPipeline.reset();
		mSphereNode->mMaterial.reset();
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
		cleanUpOffScreenResources();

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getSwapChainExtent().width;
		mHeight = mSwapChain->getSwapChainExtent().height;

		mRenderPass = Wrapper::RenderPass::create(mDevice);
		mRenderPass = createRenderPassForSwapChain();
		//mRenderPass = createRenderPass();


		mOffscreenRenderTarget = OffscreenRenderTarget::create(
			mDevice, mCommandPool,
			mWidth, mHeight,
			mSwapChain->getImageCount(),
			VK_FORMAT_R32G32B32A32_SFLOAT, // Color format
			VK_FORMAT_D24_UNORM_S8_UINT // Depth format
		);

		mSphereNode->mMaterial = Material::create();
		//mSphereNode->mMaterial->attachTexturePaths(textureFiles);
		mSphereNode->mMaterial->attachImages(mOffscreenRenderTarget->getRenderTargetImages()); // Attach the offscreen render target images to the material
		mSphereNode->mMaterial->init(mDevice, mCommandPool, mSwapChain->getImageCount());

		mPipeline = createPipeline("shaders/testVs.spv", "shaders/testFs.spv");
		mScreenQuadPipeline = createScreenQuadPipeline(mRenderPass);
		mSkyBoxPipeline = OffscreenPipeline::create(mDevice);
		mSkyBoxPipeline->build(
			mOffscreenRenderTarget->getRenderPass(),
			mWidth, mHeight,
			"shaders/SkyboxVert.spv", "shaders/SkyBoxFrag.spv",
			{ mSkyBoxNode->mUniformManager->getDescriptorLayout()->getLayout() },
			mSkyBoxNode->mModels[0]->getVertexInputBindingDescriptions(),
			mSkyBoxNode->mModels[0]->getAttributeDescriptions(),
			nullptr, // No push constants
			mDevice->getMaxUsableSampleCount(),
			VK_FRONT_FACE_CLOCKWISE
		);


		mSwapChain->createFrameBuffers(mRenderPass);


		createCommandBuffers();

		createSyncObjects();
	}

	Wrapper::RenderPass::Ptr Application::createRenderPassForSwapChain() {
		auto swapChainRenderPass = Wrapper::RenderPass::create(mDevice);
		// Create a render pass
		// 
		// 0: Final output color attachment -> PRESENT
		// 1: Resolve image(MultiSample) (render here)
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
		swapChainRenderPass->addAttachment(finalAttachment);

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
		swapChainRenderPass->addAttachment(multiAttachment);

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
		swapChainRenderPass->addAttachment(depthAttachment);

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 2;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



		// Create subpass
		Wrapper::SubPass subpass{};
		subpass.addColorAttachmentReference(multiAttachmentRef);
		subpass.setDepthStencilAttachmentReference(depthAttachmentRef);
		subpass.setResolveAttachmentReference(finalAttachmentRef);
		subpass.buildSubPassDescription();

		swapChainRenderPass->addSubpass(subpass);

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		swapChainRenderPass->addDependency(dependency);
		swapChainRenderPass->buildRenderPass();
		return swapChainRenderPass;
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
			float frameTime = GetFrameTime();
			//mModel->update();

			//mOffscreenSphereNode->mCamera.horizontalRoundRotate(GetFrameTime(), glm::vec3(0.0f), 5.0f, 30.0f);
			//mNVPMatrices.mViewMatrix = mSphereNode->mCamera.getViewMatrix();
			//mNVPMatrices.mProjectionMatrix = mSphereNode->mCamera.getProjectMatrix();
			//mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mNVPMatrices.mViewMatrix));
			//mCameraParameters.CameraWorldPosition = mSphereNode->mCamera.getCamPosition();


			//mSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mSphereNode->mModels[0]->getUniform(), mCameraParameters,mCurrentFrame);

			mOffscreenSphereNode->mCamera.horizontalRoundRotate(frameTime, glm::vec3(0.0f), 5.0f, 30.0f);
			mNVPMatrices.mViewMatrix = mOffscreenSphereNode->mCamera.getViewMatrix();
			mNVPMatrices.mProjectionMatrix = mOffscreenSphereNode->mCamera.getProjectMatrix();
			mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mOffscreenSphereNode->mModels[0]->getUniform().mModelMatrix));
			mCameraParameters.CameraWorldPosition = mOffscreenSphereNode->mCamera.getCamPosition();

			mOffscreenSphereNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mOffscreenSphereNode->mModels[0]->getUniform(), mCameraParameters, mCurrentFrame);


			mSkyBoxNode->mCamera.horizontalRoundRotate(frameTime, glm::vec3(0.0f), 5.0f, 30.0f);
			// Skybox node should always in the center of object
			mNVPMatrices.mViewMatrix = mSkyBoxNode->mCamera.getViewMatrix();
			mNVPMatrices.mProjectionMatrix = mSkyBoxNode->mCamera.getProjectMatrix();
			mNVPMatrices.mNormalMatrix = glm::transpose(glm::inverse(mSkyBoxNode->mModels[0]->getUniform().mModelMatrix));
			mCameraParameters.CameraWorldPosition = mSkyBoxNode->mCamera.getCamPosition();
			mSkyBoxNode->mModels[0]->setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(mSkyBoxNode->mCamera.getCamPosition()))); // Keep skybox at camera position to remove parallax

			mSkyBoxNode->mUniformManager->updateUniformBuffer(mNVPMatrices, mSkyBoxNode->mModels[0]->getUniform(), mCameraParameters, mCurrentFrame);


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
			
			// Render HDR to offscreen render target
			// Offscreen render pass
			VkRenderPassBeginInfo offScreenRenderPassBeginInfo{};
			offScreenRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			offScreenRenderPassBeginInfo.renderPass = mOffscreenRenderTarget->getRenderPass()->getRenderPass();
			offScreenRenderPassBeginInfo.framebuffer = mOffscreenRenderTarget->getOffScreenFramebuffers()[i];
			offScreenRenderPassBeginInfo.renderArea.offset = { 0, 0 };
			offScreenRenderPassBeginInfo.renderArea.extent = mSwapChain->getSwapChainExtent(); // should be consistent with the swap chain extent
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


			// swapchain render pass
			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = mRenderPass->getRenderPass();
			renderPassBeginInfo.framebuffer = mSwapChain->getSwapChainFramebuffers()[i];
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.renderArea.extent = mSwapChain->getSwapChainExtent();

			std::vector<VkClearValue> clearValues;


			//0: final output color attachment 1:multisample image 2: depth attachment
			VkClearValue clearFinalColor{};
			clearFinalColor.color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues.push_back(clearFinalColor);

			//1: Multisample image
			VkClearValue clearMultiSample{};
			clearMultiSample.color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues.push_back(clearMultiSample);

			//2: Depth attachment
			VkClearValue clearDepth{};
			clearDepth.depthStencil = { 1.0f, 0 };
			clearValues.push_back(clearDepth);

			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();


			// Begin command buffer
			mCommandBuffers[i]->beginCommandBuffer();

			// Begin offscreen render pass
			mCommandBuffers[i]->beginRenderPass(offScreenRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Draw the skybox
			mCommandBuffers[i]->bindGraphicPipeline(mSkyBoxPipeline->getPipeline());
			std::vector<VkDescriptorSet> skyBoxDescriptorSets = { mSkyBoxNode->mUniformManager->getDescriptorSet(mCurrentFrame) };
			mCommandBuffers[i]->bindDescriptorSets(mSkyBoxPipeline->getPipeline()->getPipelineLayout(), 0, skyBoxDescriptorSets.size(), skyBoxDescriptorSets.data());
			mSkyBoxNode->draw(mCommandBuffers[i]);
			// End skybox draw

			// Draw the offscreen sphere
			mCommandBuffers[i]->bindGraphicPipeline(mPipeline);
			std::vector<VkDescriptorSet> offscreenDescriptorSets = { mOffscreenSphereNode->mUniformManager->getDescriptorSet(mCurrentFrame) , mOffscreenSphereNode->mMaterial->getDescriptorSet(mCurrentFrame) };
			mCommandBuffers[i]->bindDescriptorSets(mPipeline->getPipelineLayout(), 0, offscreenDescriptorSets.size(), offscreenDescriptorSets.data());

			mCommandBuffers[i]->pushConstants(mPipeline->getPipelineLayout(), mPushConstantManager->getConstantParam().stageFlags,
			mPushConstantManager->getConstantParam().offset, mPushConstantManager->getConstantParam().size, &mPushConstantManager->getConstantData());

			mOffscreenSphereNode->draw(mCommandBuffers[i]);
			//

			mCommandBuffers[i]->endRenderPass();
			// End offscreen render pass

			// Begin swapchain render pass
			mCommandBuffers[i]->beginRenderPass(renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


			mCommandBuffers[i]->bindGraphicPipeline(mScreenQuadPipeline);

			std::vector<VkDescriptorSet> descriptorSets = { mSphereNode->mUniformManager->getDescriptorSet(mCurrentFrame) , mSphereNode->mMaterial->getDescriptorSet(mCurrentFrame) };
			mCommandBuffers[i]->bindDescriptorSets(mScreenQuadPipeline->getPipelineLayout(), 0, descriptorSets.size(), descriptorSets.data());


			mCommandBuffers[i]->pushConstants(mScreenQuadPipeline->getPipelineLayout(), mPushConstantManager->getConstantParam().stageFlags,
			mPushConstantManager->getConstantParam().offset, mPushConstantManager->getConstantParam().size, &mPushConstantManager->getConstantData());



			//mModel->draw(mCommandBuffers[i]);
			vkCmdDraw(mCommandBuffers[i]->getCommandBuffer(), 3, 1, 0, 0);
			// End swapchain render pass
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

		if (result == VK_ERROR_OUT_OF_DATE_KHR || mWindow->mWindowResized || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
			return;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to acquire swap chain image!");
		}

		//recordCommandBuffer(imageIndex);

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
			return;
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