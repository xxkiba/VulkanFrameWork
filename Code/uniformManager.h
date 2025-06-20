#pragma once
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/commandPool.h"
#include "base.h"

using namespace FF;

class UniformManager {
public:
	using Ptr = std::shared_ptr<UniformManager>;
	static Ptr create() {
		return std::make_shared<UniformManager>();
	}
	UniformManager();
	~UniformManager();
	void init(const Wrapper::Device::Ptr &device, const Wrapper::CommandPool::Ptr &commandPool,int frameCount);
	void build();
	void attachCubeMap(Wrapper::Image::Ptr &inImage);
	void updateUniformBuffer(const NVPMatrices &vpMatrices, const ObjectUniform &objectUniform, const cameraParameters& cameraParams, const int frameCount);

	[[nodiscard]] auto getDescriptorLayout() const {
		return mDescriptorLayout;
	}
	[[nodiscard]] auto getDescriptorPool() const  {
		return mDescriptorPool;
	}
	[[nodiscard]] auto getDescriptorSet(int frameCount) const {
		return mDescriptorSet->getDescriptorSet(frameCount);
	}
	[[nodiscard]] auto getUniformParameters() const {
		return mUniformParameters;
	}

private:
	Wrapper::Device::Ptr mDevice{ nullptr };
	Wrapper::CommandPool::Ptr mcommandpool{ nullptr };
	std::vector<Wrapper::UniformParameter::Ptr> mUniformParameters{};

	Wrapper::DescriptorSetLayout::Ptr mDescriptorLayout{ nullptr };
	Wrapper::DescriptorPool::Ptr mDescriptorPool{ nullptr };
	Wrapper::DescriptorSet::Ptr mDescriptorSet{ nullptr };
	int mFrameCount = 1;


};