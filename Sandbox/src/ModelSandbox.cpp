#include "ModelSandbox.h"
#include <print>



static void Rotate(float deltaTime, float rotateSpeed, float& yaw, float& pitch, glm::mat4& world)
{
	#define MOUSE 1
	#if MOUSE == 0
		if (Input::IsKeyPressed(VS_KEY_A)) yaw -= rotateSpeed * deltaTime; // Left
		if (Input::IsKeyPressed(VS_KEY_D)) yaw += rotateSpeed * deltaTime; // Right
		if (Input::IsKeyPressed(VS_KEY_W)) pitch += rotateSpeed * deltaTime; // Up (Loop)
		if (Input::IsKeyPressed(VS_KEY_S)) pitch -= rotateSpeed * deltaTime; // Down (Loop)
	
		// 2. Position the model at the camera/target point
	
		// 3. Rotate YAW (Horizontal)
		// We rotate around the Y-axis (0, 1, 0)
		world = glm::rotate(world, yaw, glm::vec3(0, 1, 0));
	
		// 4. Rotate PITCH (Vertical)
		// We rotate around the X-axis (1, 0, 0)
		// Because there is no clamp, this will allow full 360 vertical loops
		world = glm::rotate(world, pitch, glm::vec3(1, 0, 0));
	
		// 5. Scale
		world = glm::scale(world, glm::vec3(2.0f));
	#else
		static bool dragging = false;
		if (Input::IsMousePressed(VS_MOUSE_LEFT))
		{
			if (!dragging)
			{
				dragging = true;
			}
			else
			{
				auto  deltaX = Input::GetMouseDeltaX();
				auto  deltaY = Input::GetMouseDeltaY();
				yaw += deltaX * rotateSpeed ;
				pitch += deltaY * rotateSpeed ; 
			}
		}
		else
		{
			dragging = false;
		}
	
		
		world = glm::rotate(world, yaw, glm::vec3(0, 1, 0));
		world = glm::rotate(world, pitch, glm::vec3(1, 0, 0));
		world = glm::scale(world, glm::vec3(2.0f));
	#endif
}



#define MODEL 1
#define TEXT 1
ModelSandbox:: ModelSandbox(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
	{
#if MODEL
		m_DefaultShader = LoadProgram("model.vert", "model.frag");
		m_FontShader = LoadProgram("render_instance_quad.vert", "solid_color.frag");
		m_Model = LoadModel("DamagedHelmet/glTF-Binary/DamagedHelmet.glb");

#else
		// MO MODEL
		m_DefaultShader = LoadProgram("basic.vert", "texture.frag");
		m_VertexLayout.Add(ShaderDataType::FLOAT3)
			.Add(ShaderDataType::FLOAT2);
		auto [verts, indices] = GenerateCube<Vertex>();
		m_Cube = verts;
		m_IndexCube = indices;

		m_VertexHandle = CreateVertexBuffer({
			reinterpret_cast<uint8_t*>(m_Cube.data()),m_Cube.size() * sizeof(m_Cube[0]) }
		, m_VertexLayout);
		m_IndexHandle = CreateIndexBuffer
		(
			Memory{ reinterpret_cast<uint8_t*>(m_IndexCube.data()), m_IndexCube.size() * sizeof(m_IndexCube[0]) }
		);

		m_MorganaTex = LoadTexture("morgana.png");
#endif
		auto [verts, indices] = GeneratePlane<Vertex_>(1);

		m_VertexLayoutQuad.AddVertex(ShaderDataType::FLOAT3);
		m_VertexLayoutQuad.AddVertex(ShaderDataType::FLOAT4);
		m_VertexLayoutQuad.AddVertex(ShaderDataType::FLOAT2);

		m_Quad = verts;
		m_IndexQuad = indices;
		m_VertexQuadBuffer = CreateVertexBuffer({
			reinterpret_cast<uint8_t*>(m_Quad.data()),m_Quad.size() * sizeof(m_Quad[0]) }
		, m_VertexLayoutQuad);
		m_IndexQuadBuffer = CreateIndexBuffer
		(
			Memory{ reinterpret_cast<uint8_t*>(m_IndexQuad.data()), m_IndexQuad.size() * sizeof(m_IndexQuad[0]) }
		);
		m_Font = LoadFont("Fonts/Inter/static/Inter_24pt-Regular.ttf");
		GetCamera()->SetCameraControl(CameraControlMode::DIRECT_CONTROL);
		


		auto color = CreateAttachment(AttachmentType::COLOR, TextureFormat::RGBA16_SFLOAT,
			Application::GetScreenWidth(), Application::GetScreenHeight(), SampleCount::e1, AttachmentHint::SampledLater);
		m_UIFrameBuffer = CreateFramebuffer({color});

		GetCamera()->LookAt({ 0,0,0 });
		ExecuteFrame(0);
	}

	void ModelSandbox::Update(float deltaTime)
	{
		SetViewRect(0, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
		SetViewTransform(0, GetCamera()->GetView(), GetCamera()->GetProj());
		auto screenWidth = GetScreenWidth();
		auto screenHeight = GetScreenHeight();
#if MODEL 
		glm::mat4 world = glm::mat4(1.0f);
		glm::mat4 world2 = glm::translate(glm::mat4(1.0f), glm::vec3(1,0,0));
		glm::mat4 world3 = glm::translate(glm::mat4(1.0f), glm::vec3(3,0,0));
		glm::mat4 world4 = glm::translate(glm::mat4(1.0f), glm::vec3(5,0,0));
		static float m_Yaw = 0, m_Pitch = glm::radians(-90.0);
		static float time = 0;
		time += deltaTime;
		auto offset = glm::sin(time * 4000) ;
		auto move = offset * 3;
		glm::vec3 pos= { 0,0, 0 };
		pos.x += move;
		//std::println("{} ",pos.x);
		world = glm::translate(world, pos);
		Rotate(deltaTime,0.005,m_Yaw,m_Pitch, world);
		//auto pos = GetCamera()->GetPosition();
		//std::println("{} {} {}", pos.x, pos.y, pos.z);
		BindVertexBuffer(0, m_Model->m_VertexBuffer);
		BindIndexBuffer(m_Model->m_IndexBuffer);
		SubmitModel(m_Model, 0, 0, m_DefaultShader, { world, world2,world3, world4 });
		Submit(0, m_DefaultShader);

	
		//ExecuteFrame(deltaTime);
#if TEXT
		int width = screenWidth / 4; 
		int height = screenHeight / 4;

		glm::mat4 proj = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
		SeUIProj(1, proj);
		SeUIProj(0, proj);
		SetFramebuffer(1, m_UIFrameBuffer);
		SetViewRect(1, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
		BindVertexBuffer(0, m_VertexQuadBuffer);
		BindIndexBuffer(m_IndexQuadBuffer);
		SubmitQuad(glm::vec2(250,250),glm::vec2(50),glm::vec4(1,1,1,1));
		SubmitQuad(glm::vec2(250+250,250),glm::vec2(20),glm::vec4(1,1,1,1));
		SubmitQuad(glm::vec2(250,250+250),glm::vec2(30),glm::vec4(1,1,1,1));
		SubmitQuad(glm::vec2(250+250,250+250),glm::vec2(40),glm::vec4(1,1,1,1));
		//SubmitText("Voidstar", screenWidth - width, 0, m_Font);


		//BlendMode state;
		//state.enabled = false;
		//SetBlendState(0, state);

		SetDepthTest(false);
		Submit(1, m_FontShader);
		SetOverlay(0,1);
#endif


#else
		BindVertexBuffer(0, m_VertexHandle);
		BindIndexBuffer(m_IndexHandle);
		BindTexture("u_Texture",m_MorganaTex);
		Submit(0, m_DefaultShader);
#endif


		ExecuteFrame(deltaTime);

	}
	



