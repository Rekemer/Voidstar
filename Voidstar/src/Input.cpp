#include "Prereq.h"
#include "Input.h"
#include <glfw3.h>
#include <print>

#include "Window.h"



namespace Voidstar {

	bool Input::m_keysCurrentFrame[1024];
	bool Input::m_keysLastFrame[1024];
	bool Input::m_LastMouseStates[3];
	bool Input::m_CurrentMouseStates[3];
	bool Input::m_keysTyped[1024];

	double Input::m_MouseLastPosX = 0.0;
	double Input::m_MouseLastPosY = 0.0;
	double Input::m_MouseCurrentPosX = 0.0;
	double Input::m_MouseCurrentPosY = 0.0;

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


		glfwGetCursorPos(window->GetRaw(), &m_MouseCurrentPosX, &m_MouseCurrentPosY);
		m_MouseLastPosX = m_MouseCurrentPosX;
		m_MouseLastPosY = m_MouseCurrentPosY;

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

	bool Input::IsMouseClicked(int button) 
	{
		return (m_CurrentMouseStates[button] && !m_LastMouseStates[button]);
	}

	bool Input::IsMousePressed(int mouse) 
	{
		return glfwGetMouseButton(m_currentWindow->GetRaw(), mouse);
	}
	void Input::SetMousePos(float x, float y)
	{
		glfwSetCursorPos(m_currentWindow->GetRaw(), x, y);
	}


	double Input::GetMouseDeltaX()
	{
		return m_MouseCurrentPosX - m_MouseLastPosX;
	}
	double Input::GetMouseDeltaY()
	{
		return m_MouseCurrentPosY - m_MouseLastPosY;
	}
	std::tuple<float, float> Input::GetMousePos()
	{
		return { m_MouseCurrentPosX,m_MouseCurrentPosY };
	}
	void Input::Update()
	{
		
		for (int i = 0; i < 1024; i++)
		{
			m_keysTyped[i] = m_keysCurrentFrame[i] && !m_keysLastFrame[i];
		}
		memcpy(m_keysLastFrame, m_keysCurrentFrame, sizeof(m_keysCurrentFrame));

		for (int i = 0; i < 3; ++i) {
			m_LastMouseStates[i] = m_CurrentMouseStates[i];
			m_CurrentMouseStates[i] = glfwGetMouseButton(m_currentWindow->GetRaw(), i);
		}
		m_MouseLastPosX = m_MouseCurrentPosX;
		m_MouseLastPosY = m_MouseCurrentPosY;

		glfwGetCursorPos(m_currentWindow->GetRaw(), &m_MouseCurrentPosX, &m_MouseCurrentPosY);
	}
}


