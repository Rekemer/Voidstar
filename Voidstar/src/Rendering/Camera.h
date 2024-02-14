#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <gtc/matrix_transform.hpp>
#include "glm.hpp"

namespace Voidstar
{
	struct SphereCoords
	{
		float pitch = 0;
		float yaw = 0;
		float r = 4;
		//x = ρsinφ cosθ
		//y = ρsinφsinθ
		//z = ρcosφ
		glm::vec3 ToCart()
		{
			glm::vec3 cart;
			cart.x = r * glm::sin(yaw) * glm::cos(pitch);
			cart.y = r * glm::sin(yaw) * glm::sin(pitch);
			cart.z = r * glm::cos(yaw) ;
			return cart;
		}
	};
	class Camera
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
		glm::vec3 m_Front{ 0.0f, 0.0f, 1.0f };
		glm::vec3 m_Position{0,0,-10};
		glm::mat4 m_Proj;
		glm::mat4 m_View;
		float m_Fov = glm::radians(45.f);;
		float m_Yaw = 90.0f; // around y axis
		float m_Pitch = 0.f; // around x axis
		float speed = 20.f;
		bool m_IsControlEnabled = true;
		float width;
		float height;
		const float farPlane = 100;
		const float nearPlane = 0.00001;
	};
}