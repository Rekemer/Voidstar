#include "Prereq.h"
#include "Camera.h"
#include "../Input.h"
#include "../Keys.h"
#include "../Log.h"
#include "gtc/quaternion.hpp"
#include "gtc/quaternion.hpp"
namespace Voidstar
{

    std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
    {
        os << vec.x << ", " << vec.y << ", " << vec.z << "\n";
        return os;
    }

	void Camera::Update(float deltaTime)
	{
        if (m_IsControlEnabled)
        {
            ProcessInput(deltaTime);
            ProcessMouse();
            UpdateView();
        }
      Log::GetLog()->info("camera pos: {0} {1} {2}\n", m_Position.x, m_Position.y, m_Position.z);
        if (Input::IsKeyTyped(VS_KEY_C))
        {
            m_IsControlEnabled = !m_IsControlEnabled;
        }

	}
    void Camera::UpdateView()
    {
        glm::mat4 x = glm::inverse(glm::mat4{1,0,0,0,
                                0,-1,0,0,
                                0,0,-1,0,
                                0,0,0,1});
        m_View = x*glm::lookAt(m_Position, m_Position + m_Front, m_Up);
        auto invertedView = glm::inverse(m_View);
        //vec3(inverse(ubo.view)[3]);
       // Log::GetLog()->info("front {0} {1} {2}", m_Front.x , m_Front.y, m_Front.z);
        //m_View= glm::lookAt(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //m_View= glm::mat4(1);
       /* m_View =glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));*/
    }



    void Camera::UpdateProj(float width, float height)
    {

        float aspect = width / height;
        float inverseAspect = 1.f / aspect;
        float angle = glm::radians(45.f);
        float tan = glm::tan(angle/2);
        const float farPlane = 10000;
        const float nearPlane = 10.0;
        glm::mat4 p = {inverseAspect/tan,0,0,0,
                        0,1/tan,0,0,
                        0,0,farPlane/(farPlane - nearPlane),1,
                        0,0,(-nearPlane*farPlane)/(farPlane-nearPlane),0};
        m_Proj = glm::perspective(angle, aspect, nearPlane, farPlane);
        m_Proj[1][1] *= -1;
        m_Proj = p;

    }

  

    void Camera::ProcessInput(float deltaTime)
	{
            const float cameraSpeed = speed;
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

             auto right = glm::normalize(glm::cross(m_Front, m_Up));
            if (Input::IsKeyPressed(VS_KEY_A))
            {
                auto pos = m_Position - right * cameraSpeed * deltaTime;
                m_Position = pos;

            }

            if (Input::IsKeyPressed(VS_KEY_D))
            {
                auto pos = m_Position + right * cameraSpeed * deltaTime;
                m_Position = pos;
            }


        
	}
    bool firstMouse = true;
    double lastX = 0;
    double lastY = 0;

   // void Camera::LookAt(glm::vec3 pos)
   // {
   //
   // }

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
        float res = 90;
        float screenWidth = 16 * res;
        float screenHeight = 9 * res;
        double currentXPos = std::get<0>(position);
        double currentYPos = std::get<1>(position);
        float  clipX = (2 * currentXPos / screenWidth) - 1;
        float   clipY = 1 - (2 * currentYPos / screenHeight);


        

        float xoffset = currentXPos - lastX;
        float yoffset = currentYPos - lastY;
        yoffset = 20;

        
        
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
