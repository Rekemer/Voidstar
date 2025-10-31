#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <gtc/matrix_transform.hpp>
#include "glm.hpp"

namespace Voidstar
{
	class VOIDSTAR_API Camera
	{
	public:
		void LookAt(glm::vec3 pos);
		void Update(float deltaTime);
		inline glm::mat4& GetView() { return m_View; }
		inline glm::mat4& GetProj() { return m_Proj; }
		glm::vec3 GetPosition() { return m_Position; }
		void SetPosition(glm::vec3 pos) { m_Position = pos; }
		void SetUp(glm::vec3 up) { m_Up= up; }
		float GetFov() { return m_Fov; }
		void UpdateProj(float width,float height, float fov);
	private:
		void UpdateProj();
		void UpdateView();
		void ProcessInput(float deltaTime);
		void ProcessMouse();
		friend class Application;
		friend class Renderer;
	private:
		glm::vec3 m_Up {0.0f, 1.0f, 0.0f};
		glm::vec3 m_Front{ 0.0f, 0.0f, -1.0f };
		glm::vec3 m_Position{0,0,0};
		glm::mat4 m_Proj;
		glm::mat4 m_View;
		float m_Fov = glm::radians(45.f);;
		float m_Yaw = 90.0f; // around y axis
		float m_Pitch = 0.f; // around x axis
		float speed = 10.f;
		//bool m_IsControlEnabled = true;
		float width;
		float height;
		const float farPlane = 100;
		const float nearPlane = 0.1;
	};
}