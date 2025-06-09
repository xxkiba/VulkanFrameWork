#include "uniformManager.h"

UniformManager::UniformManager() {
}

UniformManager::~UniformManager() {

}
void UniformManager::init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount) {
	mDevice = device;
	mcommandpool = commandPool;
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

	//auto textureParam = Wrapper::UniformParameter::create();
	//textureParam->mBinding = 2;
	//textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//textureParam->mCount = 1;
	//textureParam->mStageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	////textureParam->mSize = sizeof(VkDescriptorImageInfo);
	//textureParam->mTexture = Texture::create(mDevice, mcommandpool, "assets/book.jpg");
	//mUniformParameters.push_back(textureParam);



	mDescriptorLayout = Wrapper::DescriptorSetLayout::create(device);
	mDescriptorLayout->build(mUniformParameters);

	mDescriptorPool = Wrapper::DescriptorPool::create(device);
	mDescriptorPool->build(mUniformParameters, frameCount);

	mDescriptorSet = Wrapper::DescriptorSet::create(device,mUniformParameters,mDescriptorLayout,mDescriptorPool,frameCount);

}
void UniformManager::updateUniformBuffer(const NVPMatrices& vpMatrices, const ObjectUniform& objectUniform,const int frameCount) {
	mUniformParameters[0]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&vpMatrices), sizeof(NVPMatrices));
	mUniformParameters[1]->mBuffers[frameCount]->updateBufferByMap(reinterpret_cast<const void*>(&objectUniform), sizeof(ObjectUniform));
}