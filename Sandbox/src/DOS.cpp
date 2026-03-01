#include "DOS.h"

VertexBufferHandle m_VertexHandle;
IndexBufferHandle m_IndexHandle;
ProgramHandle m_DefaultShader;
std::vector<Vertex> m_Verticies;
std::vector<IndexType> m_Indicies;
VertexLayout m_VertexLayout;
TextureHandle m_MorganaTex;

TextureHandle m_FireTexture;


DOS::DOS(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
{

	m_DefaultShader = LoadProgram("basic.vert", "texture.frag");
	m_VertexLayout.Add(ShaderDataType::FLOAT3)
		.Add(ShaderDataType::FLOAT2);
	auto [verts, indices] = GeneratePlane<Vertex>(2);
	m_Verticies = verts;
	m_Indicies = indices;

	m_VertexHandle = CreateVertexBuffer({
		reinterpret_cast<uint8_t*>(m_Verticies.data()),m_Verticies.size() * sizeof(m_Verticies[0]) }
	, m_VertexLayout);
	m_IndexHandle = CreateIndexBuffer
	(
		Memory{ reinterpret_cast<uint8_t*>(m_Indicies.data()), m_Indicies.size() * sizeof(m_Indicies[0]) }
	);

	//m_MorganaTex = LoadTexture("morgana.png");

	m_FireTexture = LoadTexture("fire/fire1_64.png");

	GetCamera()->SetCameraControl(CameraControlMode::DIRECT_CONTROL);
	GetCamera()->LookAt({ 0,0,0 });
	ExecuteFrame(0);
}

void DOS::Update(float deltaTime)
{
	SetViewRect(0, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	SetViewTransform(0, GetCamera()->GetView(), GetCamera()->GetProj());

	BindVertexBuffer(0, m_VertexHandle);
	BindIndexBuffer(m_IndexHandle);
	BindTexture("u_Texture", m_FireTexture);
	Submit(0, m_DefaultShader);
	ExecuteFrame(deltaTime);

}
