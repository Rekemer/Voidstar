#pragma once
#include "glm.hpp"
namespace Voidstar
{
	class Camera
	{
	public:
		void Update(float deltaTime);
		inline glm::mat4& GetView() { return m_View; }
		inline glm::mat4& GetProj() { return m_Proj; }

	private:
		void UpdateView();
		void UpdateProj(float width,float height);
		void ProcessInput(float deltaTime);
		void ProcessMouse();
		friend class Application;
		friend class Renderer;
	private:
		glm::vec3 m_Up {0.0f, -1.0f, 0.0f};
		glm::vec3 m_Front{ 0.0f, 0.0f, -1.0f };
		glm::vec3 m_Position{0,0,-10};
		glm::mat4 m_Proj;
		glm::mat4 m_View;
		float m_Yaw = 90.0f;
		float m_Pitch = 0.f; // up and down axis
	};
}