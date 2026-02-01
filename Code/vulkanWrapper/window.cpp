#include "window.h"
#include "../application.h"
#include "../Camera.h"

namespace FF::Wrapper {

	static void windowResized(GLFWwindow* window, int width, int height) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		pUserData->mWindowResized = true; 
	}


	static void cursorPosCallBack(GLFWwindow* window, double xpos, double ypos) {
		auto pUserData = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!pUserData->mApplication.expired()) {
			auto application = pUserData->mApplication.lock();
			application->onMouseMove(xpos, ypos);
		}
		else {
			std::cerr << "Error: application is expired" << std::endl;
		}
	}

	Window::Window(const int& width, const int& height)
	{
		mWidth = width;
		mHeight = height;
		glfwInit(); // Init glfw, once per application

		// Set environment, close opengl API 
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Allow window resize
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mWindow = glfwCreateWindow(mWidth, mHeight, "vulkan window", nullptr, nullptr);

		if (!mWindow) {
			std::cerr << "Error: failed to create window" << std::endl;
		}

		glfwSetWindowUserPointer(mWindow, this);
		glfwSetFramebufferSizeCallback(mWindow, windowResized);
		glfwSetCursorPosCallback(mWindow, cursorPosCallBack);
	}

	Window::~Window()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}
	bool Window::shouldClose()
	{
		return glfwWindowShouldClose(mWindow);
	}
	void Window::pollEvents()
	{
		glfwPollEvents();
	}
	void  Window::setApplication(const std::shared_ptr<Application>& application) {
		mApplication = application;
	}
	void Window::processEvents()
	{
		if (mApplication.expired()) {
			std::cerr << "Error: application is expired" << std::endl;
			return;
		}

		auto application = mApplication.lock();

		if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(mWindow, true);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) {
			application->onKeyPress(CAMERA_MOVE::MOVE_FRONT);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) {
			application->onKeyPress(CAMERA_MOVE::MOVE_BACK);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) {
			application->onKeyPress(CAMERA_MOVE::MOVE_LEFT);
		}

		if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) {
			application->onKeyPress(CAMERA_MOVE::MOVE_RIGHT);
		}

	}
}

