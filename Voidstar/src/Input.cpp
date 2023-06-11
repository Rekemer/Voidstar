#include "Prereq.h"
#include "Input.h"
#include <glfw3.h>

#include "Window.h"



namespace Voidstar {

	bool Input::m_keysCurrentFrame[1024];
	bool Input::m_keysLastFrame[1024];
	bool Input::m_keysTyped[1024];
	SPtr<Window> Input::m_currentWindow;
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{

		auto* pw = (Input*)glfwGetWindowUserPointer(window);
		pw->m_keysCurrentFrame[key] = action != GLFW_RELEASE;		
	}



	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		//auto event = OnScrollingEvent{ (float)yoffset };
		

		//EventSystem::OnEvent((Event&&)OnScrollingEvent { (float)yoffset });
	}
	
	void Input::Init( SPtr<Window> window)
	{
		m_currentWindow = window;
		glfwSetScrollCallback(window->GetRaw(), ScrollCallback);
		glfwSetKeyCallback(window->GetRaw(), KeyCallback);

	}
	bool Input::IsKeyPressed(int keycode)
	{
		if (keycode < 1024) {
			return m_keysCurrentFrame[keycode];
		}
	}

	bool Input::IsKeyTyped(int keycode)
	{
		if (keycode < 1024) {
			return m_keysTyped[keycode];
		}
	}

	bool Input::IsMousePressed(int mouse) 
	{
		return glfwGetMouseButton(m_currentWindow->GetRaw(), mouse);
	}
	std::tuple<float, float> Input::GetMousePos()
	{
		double currentPosX = 0, currentPosY = 0;
		glfwGetCursorPos(m_currentWindow->GetRaw(), &currentPosX, &currentPosY);
		return { currentPosX,currentPosY};
	}
	void Input::Update()
	{
		
		for (int i = 0; i < 1024; i++)
		{
			m_keysTyped[i] = m_keysCurrentFrame[i] && !m_keysLastFrame[i];
		}
		memcpy(m_keysLastFrame, m_keysCurrentFrame, sizeof(m_keysCurrentFrame));
	}
}


