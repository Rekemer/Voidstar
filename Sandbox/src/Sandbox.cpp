#include "Voidstar.h"
#include "Vertex.h"
using namespace Voidstar;



void Rotate(float deltaTime, float rotateSpeed, float& yaw, float& pitch, glm::mat4& world)
{

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
}



#define MODEL 1
class Sandbox : public Voidstar::Application
{
public:
	Sandbox(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
	{
#if MODEL
		m_DefaultShader = LoadProgram("model.vert", "texture.frag");
		m_Model = LoadModel("DamagedHelmet/glTF-Binary/DamagedHelmet.glb");
#else
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

		

		GetCamera()->SetCameraControl(CameraControlMode::NO_CONTROL);
		GetCamera()->LookAt({ 0,0,0 });
		ExecuteFrame(0);
	}

	void Update(float deltaTime)
	{
		SetViewRect(0, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
		SetViewTransform(0, GetCamera()->GetView(), GetCamera()->GetProj());

#if MODEL 
		// 1. Start with Identity
		glm::mat4 world = glm::mat4(1.0f);
		static float m_Yaw = 0, m_Pitch = 0;
		Rotate(deltaTime,1400,m_Yaw,m_Pitch, world);
		
		SubmitModel(m_Model,0,m_DefaultShader,world);
#else
		BindVertexBuffer(0, m_VertexHandle);
		BindIndexBuffer(m_IndexHandle);
		BindTexture("u_Texture",m_MorganaTex);
		Submit(0, m_DefaultShader);
#endif

		ExecuteFrame(deltaTime);

	}
	
	SPtr<Model> m_Model;
	PassID m_DefaultRenderPass;
	VertexBufferHandle m_VertexHandle;
	IndexBufferHandle m_IndexHandle;
	ProgramHandle m_DefaultShader;
	std::vector<Vertex> m_Cube;
	std::vector<IndexType> m_IndexCube;
	VertexLayout m_VertexLayout;
	TextureHandle m_MorganaTex;
};



Voidstar::Application* Voidstar::CreateApplication()
{
	auto str = std::string("Sandbox");
	// 120 -> 1920 * 1080
	// 110
	const int res = 110;
	return new Sandbox(str, std::min(16 * res, 1920), std::min(9 * res, 1061));
}

int main()
{
	return Main();
}