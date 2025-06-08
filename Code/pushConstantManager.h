#include "base.h"
#include "vulkanWrapper/constantRange.h"

namespace FF {

	struct constantParam {
		uint32_t offset{ 0 };
		uint32_t size{ 0 };
		VkShaderStageFlagBits stageFlags{ VK_SHADER_STAGE_VERTEX_BIT };
	};

	struct constantData {
		glm::mat4 constantVmatrix{ 1.0f };
		glm::mat4 constantPmatrix{ 1.0f };
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
		void updateConstantData(const glm::mat4& vMatrix, const glm::mat4& pMatrix) {
			mConstantData.constantVmatrix = vMatrix;
			mConstantData.constantPmatrix = pMatrix;
		}
		constantData& getConstantData();

	private:
		Wrapper::ConstantRange::Ptr mPushConstantRange{};
		constantParam mConstantParams{};
		constantData mConstantData{};
	};
}