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
	Application::Application(std::string_view appName ,size_t screenWidth, size_t screenHeight) :
		m_ScreenWidth{ screenWidth }, m_ScreenHeight{ screenHeight}
	{
		Log::Init();
		// init Window
		
		// init Renderer
		m_Camera = CreateSPtr<Camera>();
		m_Camera->UpdateProj(screenWidth, screenHeight,m_Camera->GetFov());

		InitParams init{screenWidth,screenHeight, this};
		init.SetName(appName);
		
		
		m_Window = CreateSPtr<Window>(init.appName, init.width, init.height);
		
		SetWindow(m_Window);
		Input::Init(m_Window);
		m_RenderThread = std::thread([this] { RunRender_(m_IsRunning); });
	
		SubmitInit(init);
		ExecuteFrame(0);
	}
	Application::~Application()
	{
		m_RenderThread.join();
		
	}
	void Application::Run()
	{
		while (m_IsRunning)
		{

			m_IsRunning = m_Window->IsClosed();
			Input::Update();
			bool shouldClose = Input::IsKeyPressed(VS_KEY_ESCAPE);
			if (shouldClose)
			{
				m_IsRunning = true;
				m_Window->Close();
				// to do shutdown signals

			}
			float deltaTime = 0;
			m_Window->Update(deltaTime);
			m_ExeTime += deltaTime;
			m_Camera->Update(deltaTime);
			Update(deltaTime);
		}
		
	}
}