#include "pushConstantManager.h"

namespace FF {
	PushConstantManager::PushConstantManager() {
	}
	PushConstantManager::~PushConstantManager() {
	}
	void PushConstantManager::init() {

		constantParam vpMatricesParam;
		vpMatricesParam.offset = 0;
		vpMatricesParam.size = sizeof(NVPMatrices);
		vpMatricesParam.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		mConstantParams=vpMatricesParam;
		mPushConstantRange= Wrapper::ConstantRange::create(vpMatricesParam.offset, vpMatricesParam.size, vpMatricesParam.stageFlags);

		mConstantData.offsets[0] = glm::vec4(-1.0f,0.0f,0.0f,0.0f);
		mConstantData.offsets[1] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		mConstantData.offsets[2] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	}

	constantData& PushConstantManager::getConstantData() {
		return mConstantData;
	}

	const Wrapper::ConstantRange::Ptr& PushConstantManager::getPushConstantRanges() {
		return mPushConstantRange;
	}
	const constantParam& PushConstantManager::getConstantParam() const {
		return mConstantParams;
	}
}