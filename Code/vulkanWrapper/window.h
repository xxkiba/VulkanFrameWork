# pragma once

#include "../base.h"


namespace FF {
	class Application;
}

namespace FF::Wrapper {
	class Window {
	public:
		using Ptr = std::shared_ptr<Window>;
		static Ptr create(const int& width, const int& height) { return std::make_shared<Window>(width,height); }

		Window(const int &width,const int &height);
		~Window();
		
		bool shouldClose();
		void pollEvents();
		void processEvents();

		void setApplication(const std::shared_ptr<Application>& application);


		[[nodiscard]] auto getWidth() const { return mWidth; }
		[[nodiscard]] auto getHeight() const { return mHeight; }
		[[nodiscard]] auto getWindow() const { return mWindow; }

	public:
		bool mWindowResized{ false };
		std::weak_ptr<Application> mApplication{};
	private:
		int mWidth{ 0 };
		int mHeight{ 0 };
		GLFWwindow* mWindow{ NULL };

		
	};
}