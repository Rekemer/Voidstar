#include "Prereq.h"
#include "Camera.h"
#include "../Input.h"
#include "../Keys.h"
#include "gtc/quaternion.hpp"
namespace Voidstar
{
	void Camera::Update(float deltaTime)
	{
        ProcessInput(deltaTime);
        ProcessMouse();
        UpdateView();
	}
    void Camera::UpdateView()
    {
        m_View = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }
    void Camera::UpdateProj(float width, float height)
    {
        m_Proj = glm::perspective(glm::radians(45.0f), width / height, 0.0001f, 100.0f);
    }

    std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
    {
        os << vec.x << ", " << vec.y << ", " << vec.z << "\n";
        return os;
    }

    void Camera::ProcessInput(float deltaTime)
	{
            const float cameraSpeed = 5.f; 
            deltaTime = 0.01f;
            if (Input::IsKeyPressed(VS_KEY_W))
            {
                auto pos = m_Position + cameraSpeed * m_Front * deltaTime;
              
                m_Position = pos;
            }

            if (Input::IsKeyPressed(VS_KEY_S))
            {
                auto pos = m_Position - cameraSpeed * m_Front * deltaTime;
                m_Position = pos;
            }

            if (Input::IsKeyPressed(VS_KEY_A))
            {
                auto pos = m_Position - glm::normalize(glm::cross(m_Front, m_Up)) * cameraSpeed * deltaTime;
                m_Position = pos;
            }

            if (Input::IsKeyPressed(VS_KEY_D))
            {
                auto pos = m_Position + glm::normalize(glm::cross(m_Front, m_Up)) * cameraSpeed * deltaTime;
                m_Position = pos;
            }


        
	}
    bool firstMouse = true;
    double lastX = 0;
    double lastY = 0;
    void Camera::ProcessMouse()
    {
       if (firstMouse)
       {
         
           auto position = Input::GetMousePos();
           lastX = std::get<0>(position);
           lastY = std::get<1>(position);
           firstMouse = false;
       }
        auto position =  Input::GetMousePos();
        double currentXPos = std::get<0>(position);
        double currentYPos = std::get<1>(position);
        float xoffset = currentXPos - lastX;
        float yoffset = currentYPos - lastY;
       // yoffset *= -1;    
        lastX = currentXPos;
        lastY = currentYPos;
        glm::vec3 direction;
        float sens = 0.4f;
        m_Yaw -= xoffset * sens;
        m_Pitch -= yoffset * sens;
        m_Pitch = glm::clamp(m_Pitch, -89.f, 89.f);
        float sq = cos(glm::radians(m_Pitch));
        direction.x = cos(glm::radians(m_Yaw)) * sq;
        direction.y = sin(glm::radians(m_Pitch));
        direction.z = sin(glm::radians(m_Yaw)) * sq;
        m_Front = glm::normalize(direction);
        lastX = currentXPos;
        lastY = currentYPos;

    }
}
