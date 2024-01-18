#include "Prereq.h"
#include "Camera.h"
#include "../Input.h"
#include "../Keys.h"
#include "../Log.h"
#include<gtc/matrix_transform.hpp>
#include<gtc/type_ptr.hpp>
#include<gtx/rotate_vector.hpp>
#include<gtx/vector_angle.hpp>
#include "glfw3.h"
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
           // LookAt({0,0,0});
        }
      //Log::GetLog()->info("camera pos: {0} {1} {2}\n", m_Position.x, m_Position.y, m_Position.z);
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
        m_View = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
        //auto invertedView = glm::inverse(m_View);
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
        const float farPlane = 10;
        const float nearPlane = 1.0;
        glm::mat4 p = {inverseAspect/tan,0,0,0,
                        0,1/tan,0,0,
                        0,0,farPlane/(farPlane - nearPlane),1,
                        0,0,(-nearPlane*farPlane)/(farPlane-nearPlane),0};
       m_Proj = glm::perspective(angle, aspect, nearPlane, farPlane);
       m_Proj[1][1] *= -1;
      // m_Proj = p;
       //m_Proj = glm::ortho(-width / 16,width/16, -height / 16,height/16,0.f,1000.f);
       //m_Proj = glm::ortho(0.0f, 800.0f,600.0f, 0.0f);
       //float zoom = 1000;
       //float aspectRatio = 9.f/16.f;
       //m_Proj = glm::ortho(0.f, zoom,zoom*aspectRatio,0.f);
       //m_Proj = glm::ortho(0.f, 16.f*90, 9.f * 90,0.f);

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

    void Camera::LookAt(glm::vec3 pos)
    {
        auto diff = glm::normalize(pos - m_Position);
        m_Front = diff;
        m_View = glm::lookAt(m_Position, pos, m_Up);
    }
   
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
        glm::vec2 currentPos = { std::get<0>(position),std::get<1>(position) };
       // float  clipX = (2 * currentXPos / screenWidth) - 1;
       // float   clipY = 1 - (2 * currentYPos / screenHeight);
        float sens = 20.f;
        float rotX = sens * (float)(currentPos.y - (screenHeight / 2)) / screenHeight;
        float rotY = sens * (float)(currentPos.x- (screenWidth / 2)) / screenWidth;
        
#if 1

        // Calculates upcoming vertical change in the Orientation
        glm::vec3 newOrientation = glm::rotate(m_Front, glm::radians(-rotX), glm::normalize(glm::cross(m_Front, m_Up)));

        // Decides whether or not the next vertical Orientation is legal or not
        if (abs(glm::angle(newOrientation, m_Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
        {
            m_Front = newOrientation;
        }

        // Rotates the Orientation left and right
        m_Front = glm::rotate(m_Front, glm::radians(-rotY), m_Up);
        // Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
        Input::SetMousePos((screenWidth / 2), (screenHeight / 2));
#else
        
#endif // 0


    }
}
