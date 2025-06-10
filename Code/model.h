#pragma once
#include "base.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"
#include "vulkanWrapper/commandBuffer.h"


namespace FF {

	//struct Vertex {
	//	glm::vec3 mPosition;
	//	glm::vec3 mColor;
	//};

	struct StaticMeshVertexData {
		glm::vec3 mPosition;
		glm::vec3 mColor;
		glm::vec2 mUV;
		glm::vec3 mNormal;
		glm::vec3 mTangent;
	};

	struct BattleFireMeshVertexData {
		glm::vec4 mPosition;
		glm::vec4 mTexcoord;
		glm::vec4 mNormal;
		glm::vec4 mTangent;
	};


	struct SubMesh {
		std::vector<uint32_t> mSubMeshIndices{};
		int mIndexCount{ 0 };
		Wrapper::Buffer::Ptr mSubMeshIndexBuffer{ nullptr };
	};


	class Model {
	public:
		using Ptr = std::shared_ptr<Model>;
		static Ptr create(const Wrapper::Device::Ptr& device) {
			return std::make_shared<Model>(device);
		}

		Model(const Wrapper::Device::Ptr& device) {
		}

		~Model() {}

		void loadModel(const std::string& path, const Wrapper::Device::Ptr& device);
		void loadBattleFireModel(const std::string& path, const Wrapper::Device::Ptr& device);

		void setVertexInputBindingDescriptions();

		std::vector<VkVertexInputBindingDescription> getVertexInputBindingDescriptions();

		void setAttributeDescription();

		std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		//[[nodiscard]] auto getVertexBuffer() const { return mVertexBuffer; }

		[[nodiscard]] auto getVertexBuffers() const {
			std::vector<VkBuffer> buffers{ mPositionBuffer->getBuffer(), mColorBuffer->getBuffer(), mUVBuffer->getBuffer() };

			return buffers;
		}

		[[nodiscard]] auto getVertexDataBuffer() const {
			std::vector<VkBuffer> buffers{ mVertexDataBuffer->getBuffer()};
			return buffers;
		};

		[[nodiscard]] auto getIndexBuffer() const { return mIndexBuffer; }

		[[nodiscard]] auto getIndexCount() const { return mIndexDatas.size(); }

		[[nodiscard]] auto getUniform() const { return mUniform; }

		void setModelMatrix(const glm::mat4 matrix) { mUniform.mModelMatrix = matrix; }
		void setVertexCount(int inVertexCount);
		void setPosition(int inIndex, float inX, float inY, float inZ);
		void setColor(int inIndex, float inR, float inG, float inB);
		void setUV(int inIndex, float inU, float inV);
		void setNormal(int inIndex, float inX, float inY, float inZ);
		void setTangent(int inIndex, float inX, float inY, float inZ);

		void update() {
			glm::mat4 rotateMatrix = glm::mat4(1.0f);
			rotateMatrix = glm::rotate(rotateMatrix, glm::radians(mAngle), glm::vec3(0.0f, 0.0f, 1.0f));
			mUniform.mModelMatrix = rotateMatrix;

			mAngle += 0.01f;
		}
		void draw(const Wrapper::CommandBuffer::Ptr& cmdBuf);

	public:
		std::vector<VkVertexInputBindingDescription> bindingDes{};
		std::vector<VkVertexInputAttributeDescription> attributeDes{};

	private:
		//std::vector<Vertex> mDatas{};
		std::vector<float> mPositions{};
		std::vector<float> mColors{};
		std::vector<unsigned int> mIndexDatas{};
		std::vector<float> mUVs{};
		std::vector<float> mNormals{};
		std::vector<float> mTangents{};

		std::unordered_map<std::string, SubMesh*> mSubMeshes{};

		std::vector<StaticMeshVertexData> mVertexDatas{};
		std::vector< BattleFireMeshVertexData> mBattleFireVertexDatas{};

		Wrapper::Buffer::Ptr mVertexDataBuffer{ nullptr };

		//Wrapper::Buffer::Ptr mVertexBuffer{ nullptr };

		Wrapper::Buffer::Ptr mPositionBuffer{ nullptr };
		Wrapper::Buffer::Ptr mColorBuffer{ nullptr };
		Wrapper::Buffer::Ptr mUVBuffer{ nullptr };

		Wrapper::Buffer::Ptr mIndexBuffer{ nullptr };


		ObjectUniform mUniform;

		float mAngle{ 0.0f };

	};
}