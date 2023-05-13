#include"Prereq.h"
#include "Application.h"
#include "Window.h"
#include "Log.h"
#include "Rendering/Renderer.h"
namespace Voidstar
{
	Application::Application(std::string appName ,size_t screenWidth, size_t screenHeight) :
		m_ScreenWidth{ screenWidth }, m_ScreenHeight{ screenHeight}
	{
		Log::Init();
		// init Window
		m_Window = new Window(appName, screenWidth, screenHeight);
		// init Renderer
		m_Renderer = new Renderer(screenWidth, screenHeight, m_Window);

	}
	Application::~Application()
	{
		delete m_Window;
		delete m_Renderer;
	}
}