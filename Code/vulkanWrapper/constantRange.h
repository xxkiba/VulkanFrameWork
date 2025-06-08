#pragma once
#include "../base.h"

namespace FF::Wrapper {
	class ConstantRange {
	public:
		using Ptr = std::shared_ptr<ConstantRange>;
		static Ptr create(uint32_t offset, uint32_t size, uint32_t stageFlags) {
			return std::make_shared<ConstantRange>(offset, size, stageFlags);
		}
		ConstantRange(uint32_t offset, uint32_t size, uint32_t stageFlags);
		~ConstantRange();
		[[nodiscard]] auto getPushConstantRange() const {
			return mPushConstantRange;
		}
		void updateConstantRange(uint32_t offset, uint32_t size, uint32_t stageFlags) {
			mPushConstantRange.offset = offset;
			mPushConstantRange.size = size;
			mPushConstantRange.stageFlags = stageFlags;
		}

	private:
		VkPushConstantRange mPushConstantRange{};
	};
}