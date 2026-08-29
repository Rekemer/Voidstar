#pragma once
#include "Prereq.h"
#include "Keys.h"
struct GLFWwindow;
namespace Voidstar
{
	class Application;
	class Window;
	class VOIDSTAR_API Input {
	public:
		static void Init(std::shared_ptr<Window> window);
		static bool IsKeyPressed(int keycode);
		static bool IsKeyTyped(int keycode);
		static bool IsMousePressed(int mouse);
		static bool IsMouseClicked(int button);
		static void SetMousePos(float x, float y);
		static double GetMouseDeltaX();
		static double GetMouseDeltaY();
		static glm::vec2 GetMousePos();
	private:
		friend static void KeyCallback(GLFWwindow* window, int key, int scancode, int action,int mods);
		friend static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void Update();
	private:
		static bool m_LastMouseStates[3];
		static bool m_CurrentMouseStates[3];
		static bool m_keysCurrentFrame[1024];
		static bool m_keysLastFrame[1024];
		static bool m_keysTyped[1024];
		static SPtr<Window> m_currentWindow;
		// Store the old position before we overwrite it
		static double m_MouseLastPosX;
		static double m_MouseLastPosY;
		static double m_MouseCurrentPosX;
		static double m_MouseCurrentPosY;
		friend class Application;

	};
}