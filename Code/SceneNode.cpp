#include "SceneNode.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace FF {
	SceneNode::SceneNode() {
		memberNeedUpdate = true; // Set to true to ensure the first draw will update the model matrix
		mPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		mRotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		mScale = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		mCamera = Camera();
	}
	SceneNode::~SceneNode() {
	}
	void SceneNode::SetPosition(float x, float y, float z) {
		mPosition = glm::vec4(x, y, z, 1.0f);
		memberNeedUpdate = true;
	}
	void SceneNode::SetRotation(float x, float y, float z) {
		mRotation = glm::vec4(x, y, z, 1.0f);
		memberNeedUpdate = true;
	}
	void SceneNode::SetScale(float x, float y, float z) {
		mScale = glm::vec4(x, y, z, 1.0f);
		memberNeedUpdate = true;
	}
	void SceneNode::draw(const Wrapper::CommandBuffer::Ptr& cmdBuf) {
		if (memberNeedUpdate) {
			Update();
			memberNeedUpdate = false;
		}
		for (const auto& model : mModels) {
			if (model) {
				//model->setModelMatrix(mModelMatrix);
				//model->getUniform().mModelMatrix = mModelMatrix;
				model->draw(cmdBuf);
			}
		}
	}
	void SceneNode::Update() {

		glm::vec3 position(mPosition);
		glm::vec3 scale(mScale);
		glm::vec3 euler(mRotation); // radians

		glm::quat q = glm::quat(euler);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), position)
			* glm::toMat4(q)
			* glm::scale(glm::mat4(1.0f), scale);

		mModelMatrix = model;
		mNormalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

		memberNeedUpdate = false;
	}
}