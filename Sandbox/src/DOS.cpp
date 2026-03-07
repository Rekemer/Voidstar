#include "DOS.h"

struct Particle
{
	glm::vec4 pos = {0,0,0,0};
	glm::vec4 lifetime;
};
VertexBufferHandle m_VertexHandle;
BufferHandle m_InstanceHandle;
IndexBufferHandle m_IndexHandle;
ProgramHandle m_DefaultShader;
std::vector<Vertex> m_Verticies;
std::vector<Particle> m_FramePerParticle;
std::vector<IndexType> m_Indicies;
VertexLayout m_VertexLayout;
VertexLayout m_InstanceLayout;
TextureHandle m_MorganaTex;
BufferHandle m_ParticleHandle;
TextureHandle m_FireTexture;
Particle* m_MappedPtr;

// if we have dynamic instance buffer we should have them per frame
DOS::DOS(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
{

	m_DefaultShader = LoadProgram("flipbook.vert", "flipbook.frag");
	m_VertexLayout.AddVertex(ShaderDataType::FLOAT3, 0)
		.AddVertex(ShaderDataType::FLOAT2, 0);

	//m_InstanceLayout.AddVertex(ShaderDataType::FLOAT, 1);
	//m_InstanceLayout.AddVertex(ShaderDataType::FLOAT, 1);
	
	auto [verts, indices] = GeneratePlane<Vertex>(2);
	m_Verticies = verts;
	m_Indicies = indices;
	m_VertexHandle = CreateVertexBuffer({
		reinterpret_cast<uint8_t*>(m_Verticies.data()),m_Verticies.size() * sizeof(m_Verticies[0]) }
	, m_VertexLayout);

	for (auto i = 0; i < 100; i++)
	{
		m_FramePerParticle.push_back({ {i * 5,0,0,0},{0,1000,0,0} });
	}
	

	m_ParticleHandle = CreateBuffer(Memory{ reinterpret_cast<uint8_t*>(m_FramePerParticle.data()), m_FramePerParticle.size() * sizeof(m_FramePerParticle[0]) },ResourceUsage::StorageRead | ResourceUsage::StorageWrite | ResourceUsage::Readback);

	//m_InstanceHandle = CreateVertexBuffer({
	//	reinterpret_cast<uint8_t*>(m_FramePerParticle.data()),m_FramePerParticle.size() * sizeof(m_FramePerParticle[0]) }
	//, m_InstanceLayout, ResourceUsage::Vertex | ResourceUsage::Upload | ResourceUsage::Readback);
	

	m_IndexHandle = CreateIndexBuffer
	(
		Memory{ reinterpret_cast<uint8_t*>(m_Indicies.data()), m_Indicies.size() * sizeof(m_Indicies[0]) }
	);

	//m_MorganaTex = LoadTexture("morgana.png");

	m_FireTexture = LoadTexture("fire/fire1_64.png");

	GetCamera()->SetCameraControl(CameraControlMode::DIRECT_CONTROL);
	GetCamera()->LookAt({ 0,0,0 });
	ExecuteFrame(0,true);


}

auto frame = 0;
float age = 0;
void DOS::Update(float deltaTime)
{
	SetViewRect(0, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	SetViewTransform(0, GetCamera()->GetView(), GetCamera()->GetProj());
	m_MappedPtr = static_cast<Particle*>(ReadMappedPtr(ResourceType::StorageBuffer, m_ParticleHandle.idx));

	

	age += deltaTime;
	m_MappedPtr->lifetime.x = age*1000;
	(m_MappedPtr+1)->lifetime.x = age*1000;

	BindVertexBuffer(0, m_VertexHandle);
	BindIndexBuffer(m_IndexHandle);
	
	BindBuffer("particles", m_ParticleHandle);
	BindTexture("u_Texture", m_FireTexture);
	
	Submit(0, m_DefaultShader, 100);
	ExecuteFrame(deltaTime);
	frame++;
	frame = frame % 255;

}
