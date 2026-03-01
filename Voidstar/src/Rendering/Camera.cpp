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
       ProcessInput(m_Mode,deltaTime);
       UpdateView();
	}
    void Camera::UpdateProj()
    {
        float aspect = width / height;
        m_Proj = glm::perspective(m_Fov, aspect, nearPlane, farPlane);
    }
    void Camera::UpdateView()
    {
        glm::mat4 x = glm::inverse(glm::mat4{1,0,0,0,
                                0,-1,0,0,
                                0,0,-1,0,
                                0,0,0,1});
        x = glm::inverse(x);
        glm::vec3 front = glm::normalize(m_Front);    // ensure non-zero
        glm::vec3 up = glm::normalize(m_Up);       // ensure non-zero
        m_View = glm::lookAtRH(m_Position, m_Position + front, up);
        
        //m_View *= x;
        //auto invertedView = glm::inverse(m_View);
        //vec3(inverse(ubo.view)[3]);
       // Log::GetLog()->info("front {0} {1} {2}", m_Front.x , m_Front.y, m_Front.z);
        //m_View= glm::lookAt(glm::vec3(0.0f, 2.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //m_View= glm::mat4(1);
       /* m_View =glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));*/
    }



    void Camera::UpdateProj(float width, float height, float fov)
    {
        this->width = width;
        this->height= height;
        m_Fov = fov;
        float aspect = width / height;
        float inverseAspect = 1.f / aspect;
        float tan = glm::tan(m_Fov /2);
        
        glm::mat4 p = {inverseAspect/tan,0,0,0,
                        0,1/tan,0,0,
                        0,0,farPlane/(farPlane - nearPlane),1,
                        0,0,(-nearPlane*farPlane)/(farPlane-nearPlane),0};
       m_Proj = p;
       m_Proj = glm::perspectiveRH_ZO(m_Fov, width / height, nearPlane, farPlane);
       //m_Proj = glm::perspective(m_Fov, aspect, nearPlane, farPlane);
       //m_Proj[1][1] *= -1;
       //m_Proj = glm::ortho(-width / 16,width/16, -height / 16,height/16,0.f,1000.f);
       //m_Proj = glm::ortho(0.0f, 800.0f,600.0f, 0.0f);
       //float zoom = 1000;
       //float aspectRatio = 9.f/16.f;
       //m_Proj = glm::ortho(0.f, zoom,zoom*aspectRatio,0.f);
       //m_Proj = glm::ortho(0.f, 16.f*90, 9.f * 90,0.f);

    }

  

    void Camera::ProcessInput(CameraControlMode mode, float deltaTime)
	{
        if (mode == CameraControlMode::NO_CONTROL) return;
        if (mode == CameraControlMode::ROUND_CONTROL)
        {
            
            float m_Radius = 15.0f;
            static float m_Yaw = 0.0f;
            static float m_Pitch = 0.0f;
            const float rotateSpeed = speed;
            const float deltaTime = 0.01f;

            // 1. Update Angles
            if (Input::IsKeyPressed(VS_KEY_A)) m_Yaw -= rotateSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_D)) m_Yaw += rotateSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_W)) m_Pitch += rotateSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_S)) m_Pitch -= rotateSpeed * deltaTime;

            // 2. REMOVED THE CLAMP
            // We no longer restrict m_Pitch to 89 degrees.

            // 3. Rebuild position
            float cp = cosf(m_Pitch);
            float sp = sinf(m_Pitch);
            float cy = cosf(m_Yaw);
            float sy = sinf(m_Yaw);

            glm::vec3 offset;
            offset.x = m_Radius * cp * cy;
            offset.y = m_Radius * sp;
            offset.z = m_Radius * cp * sy;

            m_Position = m_Target + offset;
            m_Front = glm::normalize(m_Target - m_Position);

            // 4. FIX THE UP VECTOR
            // Instead of using a fixed (0,1,0) World Up, we calculate the "Up" 
            // vector based on the current Pitch and Yaw. This prevents "Gimbal Lock."
            m_Up.x = -sp * cy;
            m_Up.y = cp;
            m_Up.z = -sp * sy;

            // Calculate Right vector normally now that Up is stable
            glm::vec3 right = glm::normalize(glm::cross(m_Front, m_Up));
        }
        else if (mode == CameraControlMode::DIRECT_CONTROL)
        {
            const float rotateSpeed = 0.5f; // Lowered: 200.0f is too fast for raw pixel deltas
            const float moveSpeed = 10.0f;
            const float deltaTime = 0.01f;

            // 1. MOUSE ROTATION
            if (Input::IsMousePressed(1)) {
                // Use the raw delta from your Mouse update logic
                m_Yaw += (float)Input::GetMouseDeltaX() * rotateSpeed * deltaTime;
                m_Pitch += (float)Input::GetMouseDeltaY() * rotateSpeed * deltaTime;
            }
            // 2. CALCULATE DIRECTION VECTORS (Based on Angles)
            // We calculate Front and Up first so we know which way to move
            float cp = cosf(m_Pitch);
            float sp = sinf(m_Pitch);
            float cy = cosf(m_Yaw);
            float sy = sinf(m_Yaw);

            m_Front.x = cp * cy;
            m_Front.y = sp;
            m_Front.z = cp * sy;
            m_Front = glm::normalize(m_Front);

            // Calculate the stable Up vector for 360-degree movement
            m_Up.x = -sp * cy;
            m_Up.y = cp;
            m_Up.z = -sp * sy;
            m_Up = glm::normalize(m_Up);

            // Calculate Right vector for strafing
            m_Right = glm::normalize(glm::cross(m_Front, m_Up));

            // 3. MOVEMENT (Update Position Directly)
            // Instead of moving a target, we move our actual location
            if (Input::IsKeyPressed(VS_KEY_W)) m_Position += m_Front * moveSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_S)) m_Position -= m_Front * moveSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_D)) m_Position += m_Right * moveSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_A)) m_Position -= m_Right * moveSpeed * deltaTime;

            // 4. THE VIEW MATRIX
            // Your Camera's LookAt should now be:
            //glm::lookAt(m_Position, m_Position + m_Front, m_Up);


        }
            
	}
    void Camera::LookAt(glm::vec3 pos)
    {
        m_Target = pos;
        auto diff = glm::normalize(pos - m_Position);
        m_Front = diff;
        m_View = glm::lookAt(m_Position, pos, m_Up);

        // tp sync angles and vectors
        m_Yaw = glm::atan(m_Front.z, m_Front.x);
        m_Pitch = glm::asin(m_Front.y);

    }
   
  

};

