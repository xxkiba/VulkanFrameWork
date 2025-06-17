#include "constantRange.h"

namespace FF::Wrapper{
	ConstantRange::ConstantRange(uint32_t offset, uint32_t size, uint32_t stageFlags) {
		mPushConstantRange.offset = offset;
		mPushConstantRange.size = size;
		mPushConstantRange.stageFlags = stageFlags;
	}
	ConstantRange::~ConstantRange() {
		// No resources to release
	}
}