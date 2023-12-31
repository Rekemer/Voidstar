#pragma once
#include"Prereq.h"
namespace Voidstar
{
	// must be passed somehow to renderer
	// for now just global variable
	
	class Renderer;
	class Window;
	class Camera;
	class Application
	{
	public:
		Application(std::string appName,size_t screenWidth, size_t screenHeight);
		void Run();

		std::unique_ptr<Camera>& GetCamera() { return m_Camera; }
		float GetExeTime() { return m_ExeTime; }
		virtual void PreRender(Camera& camera) {};
		virtual ~Application();
		size_t GetScreenWidth() { return m_ScreenWidth; }
		size_t GetScreenHeight() { return m_ScreenHeight; }
		SPtr<Window> GetWindow() { return m_Window; }
		
	private:
		friend class Renderer;
		size_t m_ScreenWidth, m_ScreenHeight;
		float m_ExeTime = 0;
		UPtr<Camera> m_Camera;
		SPtr<Window> m_Window;
	};

	Application* CreateApplication();
}

