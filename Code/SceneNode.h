#pragma once
#include "base.h"
#include "model.h"
#include "Camera.h"
#include "vulkanWrapper/commandBuffer.h"


namespace FF {
	class SceneNode {
	public:
		using Ptr = std::shared_ptr<SceneNode>;
		static Ptr create() {
			return std::make_shared<SceneNode>();
		}
		SceneNode();
		~SceneNode();
		glm::vec4 mPosition;
		glm::vec4 mRotation;
		glm::vec4 mScale;
		bool memberNeedUpdate{ false };
		glm::mat4 mModelMatrix{ 1.0f };
		glm::mat4 mNormalMatrix{ 1.0f };
		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);
		void SetScale(float x, float y, float z);
		void draw(const Wrapper::CommandBuffer::Ptr& cmdBuf);
		void Update();// Optional: js, C#, python,etc.  have this function
		std::vector<Model::Ptr> mModels{};
		Camera mCamera{};
	};
}
