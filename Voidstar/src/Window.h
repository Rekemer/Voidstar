#pragma once
struct GLFWwindow;
namespace Voidstar
{
	class Application;
	class VOIDSTAR_API  Window
	{
	public:
		size_t ScreenWidth, ScreenHeight;
		Window(Application* app,std::string windowName, size_t screenWidth, size_t screenHeight);
		GLFWwindow* GetRaw() const;
		bool IsClosed();
		void Close();
		void Update(float& deltaTime);
		~Window();
	private:
		static void ResizeCallback(GLFWwindow* window, int width, int height);
		GLFWwindow* m_Window;
		Application* m_App;
	};
}