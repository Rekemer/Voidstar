#pragma once
#include "Prereq.h"
#include <thread>
namespace Voidstar
{
	// must be passed somehow to renderer
	// for now just global variable
	
	class Renderer;
	class Window;
	class Camera;
	class VOIDSTAR_API Application
	{
	public:
		Application(std::string_view appName,size_t screenWidth, size_t screenHeight);
		void Run();

		SPtr<Camera> GetCamera() { return m_Camera; }
		float GetExeTime() { return m_ExeTime; }
		virtual void PreUpdate(Camera& camera) {};
		virtual void Update(float deltaTime) = 0;
		virtual ~Application();
		size_t GetScreenWidth() { return m_ScreenWidth; }
		size_t GetScreenHeight() { return m_ScreenHeight; }
		SPtr<Window> GetWindow() { return m_Window; }
		
	protected:
		size_t m_ActualFrameAmount;
	private:
		friend class Renderer;
		size_t m_ScreenWidth, m_ScreenHeight;
		float m_ExeTime = 0;
		std::atomic_bool m_IsRunning{ true };;
		SPtr<Camera> m_Camera;
		SPtr<Window> m_Window;
		std::thread m_RenderThread;
		
	};

	VOIDSTAR_API Application* CreateApplication();
}

