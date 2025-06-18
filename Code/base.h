#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <string>
#include <fstream>
#include <optional>
#include <unordered_map>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define GLFW_INCLUDE_VULKAN


#include <glfw/glfw3.h>
#include <vulkan/vulkan.h>

const std::vector<const char*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
};

struct NVPMatrices {
	glm::mat4 mNormalMatrix{ 1.0f };
	glm::mat4 mViewMatrix{ 1.0f };
	glm::mat4 mProjectionMatrix{ 1.0f };
};

struct ObjectUniform {
	glm::mat4 mModelMatrix{ 1.0f };
	//glm::vec4 mColor{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct cameraParameters {
	glm::vec4 CameraWorldPosition{ 0.0f, 0.0f, 0.0f, 1.0f };
};