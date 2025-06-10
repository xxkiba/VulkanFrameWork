#include "base.h"
#include "vulkanWrapper/constantRange.h"

namespace FF {

	struct constantParam {
		uint32_t offset{ 0 };
		uint32_t size{ 0 };
		VkShaderStageFlagBits stageFlags{ VK_SHADER_STAGE_VERTEX_BIT };
	};

	struct constantData {
		glm::vec4 offsets[3]{}; // For future use, currently not used
	};

	class PushConstantManager {
	public:
		using Ptr = std::shared_ptr<PushConstantManager>;
		static Ptr create() {
			return std::make_shared<PushConstantManager>();
		}
		PushConstantManager();
		~PushConstantManager();
		void init();
		const Wrapper::ConstantRange::Ptr& getPushConstantRanges();
		const constantParam& getConstantParam() const;
		void updateConstantData(const glm::vec4 offsets0,const glm::vec4 offsets1) {
			mConstantData.offsets[0] = offsets0;
			mConstantData.offsets[1] = offsets1;
		}
		constantData& getConstantData();

	private:
		Wrapper::ConstantRange::Ptr mPushConstantRange{};
		constantParam mConstantParams{};
		constantData mConstantData{};
	};
}