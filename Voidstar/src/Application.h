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
		~Application();
		void Run();
		std::unique_ptr<Camera>& GetCamera() { return m_Camera; }
		virtual void Execute() {};
	private:
		size_t m_ScreenWidth, m_ScreenHeight;

		std::unique_ptr<Camera> m_Camera;
		std::unique_ptr<Renderer> m_Renderer;
		std::shared_ptr<Window> m_Window;
	};

	Application* CreateApplication();
}

