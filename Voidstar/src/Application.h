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
		virtual ~Application();
		void Run();
		std::unique_ptr<Camera>& GetCamera() { return m_Camera; }
		virtual void Execute() {};
	private:
		friend class Renderer;
		size_t m_ScreenWidth, m_ScreenHeight;
		
		UPtr<Camera> m_Camera;
		SPtr<Window> m_Window;
	};

	Application* CreateApplication();
}

