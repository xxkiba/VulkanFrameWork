#include "uniformManager.h"

UniformManager::UniformManager() {
}

UniformManager::~UniformManager() {

}
void UniformManager::init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount) {
	mDevice = device;
	mcommandpool = commandPool;
	mFrameCount = frameCount;
	auto nvpParam = Wrapper::UniformParameter::create();
	nvpParam->mBinding = 0;
	nvpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	nvpParam->mCount = 1;
	nvpParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	nvpParam->mSize = sizeof(NVPMatrices);

	for (int i = 0; i < frameCount; i++) {
		auto buffer = Wrapper::Buffer::createUniformBuffer(device, nvpParam->mSize, nullptr);
		nvpParam->mBuffers.push_back(buffer);
	}

	mUniformParameters.push_back(nvpParam);

	auto objectParam = Wrapper::UniformParameter::create();
	objectParam->mBinding = 1;
	objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	objectParam->mCount = 1;
	objectParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	objectParam->mSize = sizeof(ObjectUniform);
	for (int i = 0; i < frameCount; i++) {
		auto buffer = Wrapper::Buffer::createUniformBuffer(device, objectParam->mSize, nullptr);
		objectParam->mBuffers.push_back(buffer);
	}
	mUniformParameters.push_back(objectParam);

	auto textureParam = Wrapper::UniformParameter::create();
	textureParam->mBinding = 2;
	textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureParam->mCount = 1;
	textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	//textureParam->mSize = sizeof(VkDescriptorImageInfo);
	std::array<std::string, 6> cubemapPaths = {
		"assets/px.jpg","assets/nx.jpg",
		"assets/py.jpg","assets/ny.jpg",
		"assets/pz.jpg","assets/nz.jpg"
	};
	textureParam->mTextures.resize(frameCount); // Resize to frameCount, each frame will have its own textures
	for (int i = 0; i < frameCount; i++) {
		auto tex = Texture::create(mDevice, mcommandpool, cubemapPaths);
			textureParam->mTextures[i].push_back(tex);
	}
	mUniformParameters.push_back(textureParam);


	auto cameraParam = Wrapper::UniformParameter::create();
	cameraParam->mBinding = 3;
	cameraParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraParam->mCount = 1;
	cameraParam->mStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	cameraParam->mSize = sizeof(cameraParameters);

	for (int i = 0; i < frameCount; i++) {
		auto buffer = Wrapper::Buffer::createUniformBuffer(device, cameraParam->mSize, nullptr);
		cameraParam->mBuffers.push_back(buffer);
	}

	mUniformParameters.push_back(cameraParam);

}

void UniformManager::attachCubeMap(Wrapper::Image::Ptr& inImage) {
	auto textureParam = Wrapper::UniformParameter::create();
	textureParam->mBinding = mUniformParameters.size(); // Use the next binding index
	textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureParam->mCount = 1;
	textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	auto cubeSampler = Wrapper::Sampler::create(mDevice, true);

	textureParam->mTextures.resize(mFrameCount); // Resize to frameCount, each frame will have its own textures
	for (int i = 0; i < mFrameCount; i++) {

		auto tex = Texture::createFromImage(mDevice, inImage, cubeSampler);
		textureParam->mTextures[i].push_back(tex);
	}
	mUniformParameters.push_back(textureParam);
}

void UniformManager::attachImage(Wrapper::Image::Ptr& inImage) {
	auto textureParam = Wrapper::UniformParameter::create();
	textureParam->mBinding = mUniformParameters.size(); // Use the next binding index
	textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureParam->mCount = 1;
	textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	auto image2DSampler = Wrapper::Sampler::create(mDevice);

	textureParam->mTextures.resize(mFrameCount); // Resize to frameCount, each frame will have its own textures
	for (int i = 0; i < mFrameCount; i++) {

		auto tex = Texture::createFromImage(mDevice, inImage, image2DSampler);
		textureParam->mTextures[i].push_back(tex);
	}
	mUniformParameters.push_back(textureParam);
}

void UniformManager::attachMapImage(Wrapper::Image::Ptr& inImage) {
	auto textureParam = Wrapper::UniformParameter::create();
	textureParam->mBinding = mUniformParameters.size(); // Use the next binding index
	textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureParam->mCount = 1;
	textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	auto MapSampler = Wrapper::Sampler::create(mDevice,false,true);

	textureParam->mTextures.resize(mFrameCount); // Resize to frameCount, each frame will have its own textures
	for (int i = 0; i < mFrameCount; i++) {

		auto tex = Texture::createFromImage(mDevice, inImage, MapSampler);
		textureParam->mTextures[i].push_back(tex);
	}
	mUniformParameters.push_back(textureParam);
}


void UniformManager::build() {
	mDescriptorLayout = Wrapper::DescriptorSetLayout::create(mDevice);
	mDescriptorLayout->build(mUniformParameters);

	mDescriptorPool = Wrapper::DescriptorPool::create(mDevice);
	mDescriptorPool->build(mUniformParameters, mFrameCount);

	mDescriptorSet = Wrapper::DescriptorSet::create(mDevice, mUniformParameters, mDescriptorLayout, mDescriptorPool, mFrameCount);
}
void UniformManager::updateUniformBuffer(const NVPMatrices& vpMatrices, const ObjectUniform& objectUniform, const cameraParameters& cameraParams, const int frameCount) {
	mUniformParameters[0]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&vpMatrices), sizeof(NVPMatrices));
	mUniformParameters[1]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&objectUniform), sizeof(ObjectUniform));

	mUniformParameters[3]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&cameraParams), sizeof(cameraParameters));
}