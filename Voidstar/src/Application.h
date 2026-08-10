#pragma once
#include "Prereq.h"
#include "Jobs.h"
#include <thread>
namespace Voidstar
{
	

	class FpsCounter
	{
	public:
		void Update(float deltaTime)
		{
			m_FrameCount++;
			m_ElapsedTime += (deltaTime * 1000);

			if (m_ElapsedTime >= m_UpdateInterval)
			{
				m_CurrentFps = m_FrameCount / m_ElapsedTime;
				m_FrameCount = 0;
				m_ElapsedTime = 0.0f;
			}
		}

		float GetFps() const { return m_CurrentFps; }
		float GetFrameTimeMs() const { return m_CurrentFps > 0.0f ? 1000.0f / m_CurrentFps : 0.0f; }

	private:
		friend class Application;
		int m_FrameCount = 0;
		float m_ElapsedTime = 0.0f;
		float m_CurrentFps = 0.0f;
		const float m_UpdateInterval = 1.0f; // recompute once per second
	};

	
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
		size_t GetScreenWidth();
		size_t GetScreenHeight();
		SPtr<Window> GetWindow() { return m_Window; }
		size_t GetFrameNumber() {
			return m_FpsCounter.m_FrameCount;
		};
	protected:
		size_t m_ActualFrameAmount;
		JobSystem m_Jobs;
	private:
		friend class Renderer;
		float m_ExeTime = 0;
		std::atomic_bool m_IsRunning{ true };
		std::atomic<size_t> m_CurrentFrame;
		FpsCounter m_FpsCounter;
		SPtr<Camera> m_Camera;
		SPtr<Window> m_Window;
		std::thread m_RenderThread;
	};

	VOIDSTAR_API Application* CreateApplication();
}

