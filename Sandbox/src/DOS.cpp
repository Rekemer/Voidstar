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
ProgramHandle m_DebugShader;
std::vector<Vertex> m_Verticies;
std::vector<Particle> m_FramePerParticle;
std::vector<IndexType> m_Indicies;
VertexLayout m_VertexLayout;
VertexLayout m_InstanceLayout;
TextureHandle m_MorganaTex;
BufferHandle m_ParticleHandle;
TextureHandle m_FireTexture;
TextureHandle m_StoneTexture;


FrameBufferHandle m_SurfaceMask;
AttachmentHandle m_SurfaceAttachment;

Particle* m_MappedPtr;
glm::mat4 world;
PassID GrondPass = 0;
PassID Flipbook = 1;
#define FLIPBOOK 0

std::vector<uint8_t> maskData;
float planeHalfSize = 5;
float planeSize = 10;
float maskRes = 1024.f;
float r = 10;
void AddSplat(int pixelX,int pixelY, float radius) {
	
	// 2. Iterate pixels in a bounding box around the hit
	for (int y = pixelY - r; y < pixelY + r; ++y) {
		for (int x = pixelX - r; x < pixelX + r; ++x) {
			float dist = glm::distance(glm::vec2(x, y), glm::vec2(pixelX, pixelY));
			if (dist < radius) {
				maskData[y * maskRes + x] = 255; 
			}
		}
	}
	
}


DOS::DOS(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
{

	m_GroundShader = LoadProgram("basic.vert", "texture.frag");
	m_DebugShader = LoadProgram("screen.vert", "render_attachment.frag");
	m_VertexLayout.AddVertex(ShaderDataType::FLOAT3, 0)
		.AddVertex(ShaderDataType::FLOAT2, 0);
	
	auto [verts, indices] = GeneratePlane<Vertex>(2);
	m_Verticies = verts;
	m_Indicies = indices;
	m_VertexHandle = CreateVertexBuffer({
		reinterpret_cast<uint8_t*>(m_Verticies.data()),m_Verticies.size() * sizeof(m_Verticies[0]) }
	, m_VertexLayout);

#if FLIPBOOK
	m_DefaultShader = LoadProgram("flipbook.vert", "flipbook.frag");
	m_FramePerParticle = { {{ 0,0,0,0 },{0,1000,0,0} } };

	m_ParticleHandle = CreateBuffer(Memory{ reinterpret_cast<uint8_t*>(m_FramePerParticle.data()), m_FramePerParticle.size() * sizeof(m_FramePerParticle[0]) },ResourceUsage::StorageRead | ResourceUsage::StorageWrite | ResourceUsage::Readback);
#endif


	m_IndexHandle = CreateIndexBuffer
	(
		Memory{ reinterpret_cast<uint8_t*>(m_Indicies.data()), m_Indicies.size() * sizeof(m_Indicies[0]) }
	);
	maskData = std::vector<uint8_t>(1024 * 1024);
	m_SurfaceAttachment = CreateAttachment(AttachmentType::COLOR,TextureFormat::R8_UNORM,1024,1024,SampleCount::e1,
		AttachmentHint::ResolveDst | AttachmentHint::ResolveSrc
		| AttachmentHint::SampledLater);
	m_SurfaceMask = CreateFramebuffer({m_SurfaceAttachment});
	//m_MorganaTex = LoadTexture("morgana.png");
#if FLIPBOOK 
	m_FireTexture = LoadTexture("fire/fire1_64.png");
#endif
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

struct Ray
{
	glm::vec3 origin;
	glm::vec3 dir; // must be normalized
};

Ray ScreenPointToRay(
	float mouseX,
	float mouseY,
	float screenWidth,
	float screenHeight,
	const glm::mat4& view,
	const glm::mat4& proj)
{
	// 1. Convert Mouse to NDC [-1, 1]
	float x = (2.0f * mouseX) / screenWidth - 1.0f;

	// VULKAN TROUBLESPOT: 
	// If your Y-axis is inverted in the viewport, use this:
	float y = (2.0f * mouseY) / screenHeight - 1.0f;
	// IF THAT HITS THE WRONG VERTICAL SPOT, USE: float y = 1.0f - (2.0f * mouseY) / screenHeight;

	// 2. Map to Clip Space
	// Vulkan Depth is [0, 1]. We use 0.0 for near, 1.0 for far.
	glm::vec4 rayStartNDC(x, y, 0.0f, 1.0f);
	glm::vec4 rayEndNDC(x, y, 1.0f, 1.0f);

	// 3. Unproject using the Inverse View-Projection
	glm::mat4 invVP = glm::inverse(proj * view);

	glm::vec4 worldStart = invVP * rayStartNDC;
	glm::vec4 worldEnd = invVP * rayEndNDC;

	// 4. Critical: Divide by W
	worldStart /= worldStart.w;
	worldEnd /= worldEnd.w;

	Ray ray{};
	ray.origin = glm::vec3(worldStart);
	ray.dir = glm::normalize(glm::vec3(worldEnd - worldStart));
	return ray;
}
bool IntersectPlane(const Ray& worldRay, const glm::mat4& model, glm::vec3& outHit)
{
	glm::mat4 invModel = glm::inverse(model);

	// Transform Ray to Local Space
	glm::vec3 p0 = glm::vec3(invModel * glm::vec4(worldRay.origin, 1.0f));
	// Transform direction as a vector (w=0)
	glm::vec3 d = glm::normalize(glm::vec3(invModel * glm::vec4(worldRay.dir, 0.0f)));

	// Your plane is at Z=0. The normal is (0, 0, 1).
	// Equation: p0.z + t * d.z = 0  =>  t = -p0.z / d.z
	if (std::abs(d.z) < 0.00001f) return false; // Parallel

	float t = -p0.z / d.z;
	if (t < 0.0f) return false; // Intersection is behind the ray origin

	glm::vec3 localHit = p0 + d * t;

	// Your GeneratePlane generates vertices from -1 to 1.
	// Ensure this matches your model scale! 
	if (localHit.x >= -1.0f && localHit.x <= 1.0f &&
		localHit.y >= -1.0f && localHit.y <= 1.0f)
	{
		outHit = glm::vec3(glm::vec4(localHit, 1.0f));
		return true;
	}

	return false;
}

void DOS::Update(float deltaTime)
{
	SetViewRect(GrondPass, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	SetViewTransform(GrondPass, GetCamera()->GetView(), GetCamera()->GetProj());
	SetTransform(world);
	//SetTransform(glm::translate(world, glm::vec3(10,0,0)));
	BindTexture("u_Texture", m_StoneTexture);
	BindVertexBuffer(0, m_VertexHandle);
	BindIndexBuffer(m_IndexHandle);
	Submit(GrondPass, m_GroundShader, 1);

	auto mousePos = Input::GetMousePos();
	Ray ray = ScreenPointToRay(std::get<0>(mousePos), std::get<1>(mousePos), Application::GetScreenWidth(), Application::GetScreenHeight(), GetCamera()->GetView(), GetCamera()->GetProj());

	glm::vec3 worldHit;
	glm::vec3 localHit;
	bool hit = IntersectPlane(
		ray,
		world,
		localHit
	);
	
	// 1. World -> UV [0, 1]
	float u = (localHit.x + 1.0f) * 0.5f;
	float v = (localHit.y + 1.0f) * 0.5f;

	// 2. UV -> Pixel [0, maskRes]
	int pixelX = static_cast<int>(u * (maskRes - 1));
	int pixelY = static_cast<int>(v * (maskRes - 1));

	//worldHit =  GetCamera()->GetPosition();
	std::cout << pixelX << " " << pixelY << "\n";
	std::cout << hit << std::endl;

	if (hit && Input::IsMousePressed(0))
	{
		AddSplat(pixelX, pixelY,300);
	}

	auto texture = GetColorTexture(m_SurfaceMask);
	
	UpdateTexture(texture, maskData.data(), sizeof(maskData[0])*maskData.size());
	
	int screenWidth = Application::GetScreenWidth();
	int screenHeight = Application::GetScreenHeight();
	int width = 500;
	int height = 400;
	SetClipRect(screenWidth - width, screenHeight - height, width, height);
	SetRenderMode(RenderMode::SCREEN);

	BindAttachmentAsTexture("u_Scene1", texture);

	Submit(GrondPass, m_DebugShader, 1);

#if FLIPBOOK
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
