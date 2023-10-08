#include"Prereq.h"
#include "Application.h"
#include "Window.h"
#include "Log.h"
#include "Rendering/Renderer.h"
#include "Input.h"

#include "Rendering/Camera.h"
namespace Voidstar
{
	Application::Application(std::string appName ,size_t screenWidth, size_t screenHeight) :
		m_ScreenWidth{ screenWidth }, m_ScreenHeight{ screenHeight}
	{
		Log::Init();
		// init Window
		m_Window = CreateSPtr<Window>(appName, screenWidth, screenHeight);
		// init Renderer
		m_Camera = CreateUPtr<Camera>();
		m_Camera->UpdateProj(screenWidth, screenHeight);
		Input::Init(m_Window);
		Renderer::Instance()->Init(screenWidth, screenHeight, m_Window, this);

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
			Renderer::Instance()->Render(deltaTime,*m_Camera);
		}
		
	}
}