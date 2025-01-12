#pragma once
#include "Prereq.h"
#include "Keys.h"
struct GLFWwindow;
namespace Voidstar
{

	

		class Renderer2D;
		class Application;
		class Window;
		class  Input {
		public:
			static void Init(std::shared_ptr<Window> window);
			static bool IsKeyPressed(int keycode);
			static bool IsKeyTyped(int keycode);
			static bool IsMousePressed(int mouse);
			static void SetMousePos(float x, float y);
			static std::tuple<float,float> GetMousePos();
		private:
			friend static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
			friend static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
			static void Update();
		private:
			static bool m_keysCurrentFrame[1024];
			static bool m_keysLastFrame[1024];
			static bool m_keysTyped[1024];
			static SPtr<Window> m_currentWindow;

			friend class Application;

		};
}