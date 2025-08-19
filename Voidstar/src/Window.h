#pragma once
struct GLFWwindow;
namespace Voidstar
{
	class VOIDSTAR_API  Window
	{
	public:
		Window(std::string windowName, size_t screenWidth, size_t screenHeight);
		GLFWwindow* GetRaw() const;
		bool IsClosed();
		void Close();
		void Update(float& deltaTime);
		~Window();
	private:
		GLFWwindow* m_Window;
	};
}