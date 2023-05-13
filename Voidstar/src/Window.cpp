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
		if (!m_Window)
		{
			Log::GetLog()->error("Window is not created!");
		}
	}

	 GLFWwindow* Window::GetRaw() const {
		return m_Window;
	}


	Window::~Window()
	{
		glfwTerminate();
	}
}