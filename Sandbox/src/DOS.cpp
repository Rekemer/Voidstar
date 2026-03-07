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
ProgramHandle m_GroundShader;
std::vector<Vertex> m_Verticies;
std::vector<Particle> m_FramePerParticle;
std::vector<IndexType> m_Indicies;
VertexLayout m_VertexLayout;
VertexLayout m_InstanceLayout;
TextureHandle m_MorganaTex;
BufferHandle m_ParticleHandle;
TextureHandle m_FireTexture;
TextureHandle m_StoneTexture;
Particle* m_MappedPtr;

glm::mat4 world;
// if we have dynamic instance buffer we should have them per frame
DOS::DOS(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
{

	m_DefaultShader = LoadProgram("flipbook.vert", "flipbook.frag");
	m_GroundShader = LoadProgram("basic.vert", "texture.frag");
	m_VertexLayout.AddVertex(ShaderDataType::FLOAT3, 0)
		.AddVertex(ShaderDataType::FLOAT2, 0);
	
	auto [verts, indices] = GeneratePlane<Vertex>(2);
	m_Verticies = verts;
	m_Indicies = indices;
	m_VertexHandle = CreateVertexBuffer({
		reinterpret_cast<uint8_t*>(m_Verticies.data()),m_Verticies.size() * sizeof(m_Verticies[0]) }
	, m_VertexLayout);

	m_FramePerParticle = { {{ 0,0,0,0 },{0,1000,0,0} } };

	m_ParticleHandle = CreateBuffer(Memory{ reinterpret_cast<uint8_t*>(m_FramePerParticle.data()), m_FramePerParticle.size() * sizeof(m_FramePerParticle[0]) },ResourceUsage::StorageRead | ResourceUsage::StorageWrite | ResourceUsage::Readback);


	m_IndexHandle = CreateIndexBuffer
	(
		Memory{ reinterpret_cast<uint8_t*>(m_Indicies.data()), m_Indicies.size() * sizeof(m_Indicies[0]) }
	);

	//m_MorganaTex = LoadTexture("morgana.png");

	m_FireTexture = LoadTexture("fire/fire1_64.png");
	m_StoneTexture = LoadTexture("Cobblestone.png");

	GetCamera()->SetPosition({ 17,-14,17 });
	GetCamera()->SetCameraControl(CameraControlMode::DIRECT_CONTROL);
	GetCamera()->LookAt({ 0,0,0 });
	ExecuteFrame(0,true);

	world = glm::mat4(1);
	world = glm::rotate(world, glm::radians(90.f), glm::vec3(1, 0, 0));
	world = glm::scale(world, glm::vec3(10.0f));
}

auto frame = 0;
float age = 0;
void DOS::Update(float deltaTime)
{
	//SetViewRect(0, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	//SetViewTransform(0, GetCamera()->GetView(), GetCamera()->GetProj());
	//SetTransform(world);
	////SetTransform(glm::translate(world, glm::vec3(10,0,0)));
	//BindTexture("u_Texture", m_StoneTexture);
	//BindVertexBuffer(0, m_VertexHandle);
	//BindIndexBuffer(m_IndexHandle);
	//Submit(0, m_GroundShader, 1);

#if 1
	SetViewRect(0, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	SetViewTransform(0, GetCamera()->GetView(), GetCamera()->GetProj());
	m_MappedPtr = static_cast<Particle*>(ReadMappedPtr(ResourceType::StorageBuffer, m_ParticleHandle.idx));

	age += deltaTime;
	m_MappedPtr->lifetime.x = age*1000;
	//(m_MappedPtr+1)->lifetime.x = age*1000;
	
	BindVertexBuffer(0, m_VertexHandle);
	BindIndexBuffer(m_IndexHandle);
	
	BindTexture("u_Texture", m_FireTexture);
	BindBuffer("particles", m_ParticleHandle);
	
	Submit(0, m_DefaultShader, 1);
	frame++;
	frame = frame % 255;
#endif
	ExecuteFrame(deltaTime);

}
