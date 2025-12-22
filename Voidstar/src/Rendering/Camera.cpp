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
       //ProcessMouse();
       UpdateView();
      //Log::GetLog()->info("camera pos: {0} {1} {2}\n", m_Position.x, m_Position.y, m_Position.z);
        //if (Input::IsKeyTyped(VS_KEY_C))
        //{
        //    m_IsControlEnabled = !m_IsControlEnabled;
        //}
       // auto delta = .125/6;
       // if (Input::IsKeyPressed(VS_KEY_F))
       // {
       //
       //     m_Fov -= delta;
       //     UpdateProj();
       // }
       // if (Input::IsKeyPressed(VS_KEY_G))
       // {
       //
       //     m_Fov += delta;
       //     UpdateProj();
       // }

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
            
            float m_Radius = 5.0f;                 // distance to target
            static float m_Yaw = 0.0f;                  // radians
            static float m_Pitch = 0.0f;                  // radians
            const float rotateSpeed = speed;      // radians/sec-ish
            const float zoomSpeed = speed;      // units/sec
            // use your real deltaTime, don't hardcode it
            // deltaTime = ...
            deltaTime = 0.01f;
           
           // yaw: left/right
            if (Input::IsKeyPressed(VS_KEY_A)) m_Yaw -= rotateSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_D)) m_Yaw += rotateSpeed * deltaTime;

            // pitch: up/down  (THIS is what you asked)
            if (Input::IsKeyPressed(VS_KEY_W)) m_Pitch += rotateSpeed * deltaTime;
            if (Input::IsKeyPressed(VS_KEY_S)) m_Pitch -= rotateSpeed * deltaTime;

            // clamp pitch to avoid flipping at the poles
            const float pitchLimit = glm::radians(89.0f);
            m_Pitch = glm::clamp(m_Pitch, -pitchLimit, pitchLimit);

            // rebuild camera position from yaw/pitch/radius
            float cp = cosf(m_Pitch);
            glm::vec3 offset;
            offset.x = m_Radius * cp * cosf(m_Yaw);
            offset.y = m_Radius * sinf(m_Pitch);
            offset.z = m_Radius * cp * sinf(m_Yaw);

            m_Position = m_Target + offset;

            // look at target
            m_Front = glm::normalize(m_Target - m_Position);

            // stable up
            glm::vec3 worldUp = glm::vec3(0, 1, 0);
            glm::vec3 right = glm::normalize(glm::cross(m_Front, worldUp));
            m_Up = glm::normalize(glm::cross(right, m_Front));
        }
        else
        {
            const float cameraSpeed = speed;
            deltaTime = 0.01f;
            if (Input::IsKeyPressed(VS_KEY_W))
            {
                auto pos = m_Position + cameraSpeed * m_Front * deltaTime / 2.f;

                m_Position = pos;
            }

            if (Input::IsKeyPressed(VS_KEY_S))
            {
                auto pos = m_Position - cameraSpeed * m_Front * deltaTime / 2.f;
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
            
	}
    bool firstMouse = true;
    double lastX = 0;
    double lastY = 0;

    void Camera::LookAt(glm::vec3 pos)
    {
        m_Target = pos;
        auto diff = glm::normalize(pos - m_Position);
        m_Front = diff;
        m_View = glm::lookAt(m_Position, pos, m_Up);
    }
   
    void Camera::ProcessMouse() {
        auto [mx, my] = Input::GetMousePos();

        if (firstMouse) { lastX = mx; lastY = my; firstMouse = false; return; }

        float dx = mx - lastX;         // right  = positive
        float dy = my - lastY;         // down   = positive (screen origin top-left)
        lastX = mx; lastY = my;
        float sens = 0.54;
        // Signs you can flip if it "feels" wrong:
        m_Yaw += sens * dx;            // invert yaw? change to 'yaw -= sens * dx'
        m_Pitch += sens * dy;            // invert pitch? change to 'pitch += sens * dy'

        // Clamp pitch (avoid gimbal flip)
        m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

        // Recompute forward from yaw/pitch (RH, -Z forward when yaw=-90)
        float cy = glm::cos(glm::radians(m_Yaw));
        float sy = glm::sin(glm::radians(m_Yaw));
        float cp = glm::cos(glm::radians(m_Pitch));
        float sp = glm::sin(glm::radians(m_Pitch));

        m_Front = glm::normalize(glm::vec3(cy * cp, sp, sy * cp));
        // If your world is Z-up or something exotic, adjust this math accordingly.
    }

};

