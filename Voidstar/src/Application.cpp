#include"Prereq.h"
#include "Application.h"
#include "Window.h"
#include "Log.h"
#include "Rendering/Renderer.h"
#include "Input.h"
#include "Rendering/Camera.h"
#include "Submission.h"
#include "InitParams.h"
#include <thread>

namespace Voidstar
{

	size_t Application::GetScreenWidth() { return m_Window->ScreenWidth; }
	size_t Application::GetScreenHeight() { return m_Window->ScreenHeight; }

	Application::Application(std::string_view appName ,size_t screenWidth, size_t screenHeight)
	{
		Log::Init();
		// init Window
		
		// init Renderer
		m_Camera = CreateSPtr<Camera>();
		m_Camera->UpdateProj(screenWidth, screenHeight,m_Camera->GetFov());

		InitParams init{screenWidth,screenHeight, this};
		init.SetName(appName);
		
		
		m_Window = CreateSPtr<Window>(this,init.appName, init.width, init.height);
		
		SetWindow(m_Window);
		Input::Init(m_Window);
#if THREADING
		Log::GetLog()->debug("Threading is enabled");
		m_RenderThread = std::thread([this] { RunRender_(m_IsRunning); });
#endif
		m_Jobs.Start();
		SubmitInit(init);
		ExecuteFrame(0,true);

		std::cout << BASE_RES_PATH << std::endl;
	}
	Application::~Application()
	{
#if THREADING
		m_RenderThread.join();
#endif 
		
	}
	void Application::Run()
	{
		while (m_IsRunning)
		{

			m_IsRunning = !(m_Window->IsClosed() || Input::IsKeyPressed(VS_KEY_ESCAPE));


			if (!m_IsRunning)
			{
				m_Window->Close();
				// in case render thread sleeps
				#if THREADING
				WakeUpRender_();
				#endif 
				break;
			}
			float deltaTime = 0;
			m_Window->Update(deltaTime);
			Input::Update();
			m_ExeTime += deltaTime;
			m_Camera->Update(deltaTime);
			Update(deltaTime);
		}
	}
}