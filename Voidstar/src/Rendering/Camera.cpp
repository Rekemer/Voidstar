#include "Prereq.h"
#include "Camera.h"
#include "../Input.h"
#include "../Keys.h"
#include "../Log.h"
#include "gtc/quaternion.hpp"
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
        //m_View= glm::lookAt(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //m_View= glm::mat4(1);
       /* m_View =glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));*/
    }


    glm::mat4 PreparePerspectiveProjectionMatrix(float aspect_ratio,
        float field_of_view,
        float near_plane,
        float far_plane)
    {

        const float tanHalfFovy = glm::tan(field_of_view / 2.f);

        auto projectionMatrix = glm::mat4{ 0.0f };
        projectionMatrix[0][0] = 1.f / (aspect_ratio * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = far_plane / (far_plane - near_plane);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(far_plane * near_plane) / (far_plane - near_plane);

       // return projectionMatrix;

        float f = glm::tan(glm::radians(0.5f * field_of_view));
        auto inverse_aspec = 1.f / aspect_ratio;
        glm::mat4 perspective_projection_matrix = {
          inverse_aspec / f,
          0.0f,
          0.0f,
          0.0f,

          0.0f,
          1 / f,
          0.0f,
          0.0f,

          0.0f,
          0.0f,
          far_plane / (far_plane - near_plane),
          1.0f,

          0.0f,
          0.0f,
          (-1 * near_plane*far_plane) / (far_plane - near_plane),
          0.0f
        };
        return perspective_projection_matrix;
    }

    void Camera::UpdateProj(float width, float height)
    {

       /* m_Proj = PreparePerspectiveProjectionMatrix(width / height, 45.0f,0,100);*/

        m_Proj = glm::perspective(glm::radians(45.0f), width / height, 0.0001f, 100.0f);
        m_Proj[1][1] *= -1;

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
                auto right = glm::normalize(glm::cross(m_Front, m_Up));
                auto pos = m_Position - right * cameraSpeed * deltaTime;
                m_Position = pos;

            }

            if (Input::IsKeyPressed(VS_KEY_D))
            {
                auto right =glm::normalize(glm::cross(m_Front, m_Up));
                auto pos = m_Position + right * cameraSpeed * deltaTime;
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

        
        
        lastX = currentXPos;
        lastY = currentYPos;
        glm::vec3 direction;
        float sens = 0.4f;
        m_Yaw += xoffset * sens;
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
