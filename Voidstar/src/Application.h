#pragma once
#include"Prereq.h"
namespace Voidstar
{
	class Renderer;
	class Window;
	class Camera;
	class Application
	{
	public:
		Application(std::string appName,size_t screenWidth, size_t screenHeight);
		void Run();

		std::unique_ptr<Camera>& GetCamera() { return m_Camera; }
		size_t GetExeTime() { return m_ExeTime; }
		virtual void Execute() {};
		virtual ~Application();
	private:
		friend class Renderer;
		size_t m_ScreenWidth, m_ScreenHeight;
		size_t m_ExeTime = 0;
		UPtr<Camera> m_Camera;
		SPtr<Window> m_Window;
	};

	Application* CreateApplication();
}

