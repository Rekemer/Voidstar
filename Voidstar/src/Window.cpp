#include"Prereq.h"
#include"Window.h"
#include"Log.h"
#include"glfw3.h"
namespace Voidstar
{
	Window::Window(std::string windowName, size_t screenWidth, size_t screenHeight)
	{
		glfwInit();

		//no default rendering client, we'll hook vulkan up
		//to the window later
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//resizing breaks the swapchain, we'll disable it for now
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(screenWidth, screenHeight, windowName.c_str(), nullptr, nullptr);

		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (!m_Window)
		{
			Log::GetLog()->error("Window is not created!");
		}
	}

	 GLFWwindow* Window::GetRaw() const {
		return m_Window;
	}

	 bool Window::IsClosed()
	 {
		 return glfwWindowShouldClose(m_Window);
	 }

	 void Window::Close()
	 {
		 glfwSetWindowShouldClose(m_Window, GL_TRUE);
	 }

    float lastFrameTimeStart = 0;
	 void Window::Update(float& deltaTime)
	 {
		 float timeAppStart = (float)glfwGetTime();
		 deltaTime = (timeAppStart - lastFrameTimeStart) / 1000.0f;
		 lastFrameTimeStart = timeAppStart;
		 glfwPollEvents();
	 }


	Window::~Window()
	{
		glfwTerminate();
	}
}