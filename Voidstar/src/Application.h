#pragma once
#include"Prereq.h"
namespace Voidstar
{
	class Renderer;
	class Window;
	class Application
	{
	public:
		Application(std::string appName,size_t screenWidth, size_t screenHeight);
		~Application();
		void Run();
		virtual void Execute() {};
	private:
		size_t m_ScreenWidth, m_ScreenHeight;
		//std::unique_ptr<Renderer> m_Renderer;
		Renderer* m_Renderer;
		Window* m_Window;
	};

	Application* CreateApplication();
}

