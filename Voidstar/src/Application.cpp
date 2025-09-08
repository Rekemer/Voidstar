#include"Prereq.h"
#include "Application.h"
#include "Window.h"
#include "Log.h"
#include "Rendering/Renderer.h"
#include "Input.h"
#include "Rendering/Camera.h"
#include "Submission.h"
#include "InitParams.h"

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

		InitParams init{screenWidth,screenHeight};
		init.SetName(appName);
		
		
		m_Window = CreateSPtr<Window>(init.appName, init.width, init.height);
		
		SetWindow(m_Window);
		Input::Init(m_Window);
		SubmitInit(init);
		Frame();

	}
	Application::~Application()
	{
		
		
	}
	void Application::Run()
	{
		while (!m_Window->IsClosed())
		{
			Input::Update();
			bool shouldClose = Input::IsKeyPressed(VS_KEY_ESCAPE);
			if (shouldClose)
			{
				m_Window->Close();
			}
			float deltaTime = 0;
			m_Window->Update(deltaTime);
			m_ExeTime += deltaTime;
			m_Camera->Update(deltaTime);

			//PreUpdate(*m_Camera);
			Update(deltaTime);

			//Renderer::Instance()->BeginFrame(*m_Camera,m_ScreenWidth,m_ScreenHeight);
			//Renderer::Instance()->Render(deltaTime,*m_Camera);
			//Renderer::Instance()->EndFrame();
		}
		
	}
}