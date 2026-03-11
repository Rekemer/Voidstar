#include "DOS.h"
#include <random>

struct Particle
{
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
TextureHandle NoiseTexture;
BufferHandle m_ParticleHandle;
TextureHandle m_FireTexture;
TextureHandle m_StoneTexture;


FrameBufferHandle m_SurfaceMask;
AttachmentHandle m_SurfaceAttachment;

Particle* m_MappedPtr;
glm::mat4 world;
PassID GrondPass = 0;
PassID Flipbook = 1;
#define FLIPBOOK 1
#define GROUND 0

std::vector<uint8_t> maskData;
std::vector<uint8_t> blurredMaskData;
std::vector<glm::mat4> fireWorlds;

float planeHalfSize = 5;
float planeSize = 10;

int gridH = 1024/2;
int gridW = 1024/2;
// Returns a random integer in the range [min, max] (inclusive)
int RandomRangeInt(int min, int max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(min, max);
	return dist(gen);
}

// Returns a random float in the range [min, max] (inclusive)
float RandomRange(float min, float max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}

void AddInverseSplat(int cx, int cy, float radius)
{
	float rSq = radius * radius;
	for (int y = cy - radius; y <= cy + radius; ++y)
	{
		for (int x = cx - radius; x <= cx + radius; ++x)
		{
			float distSq = (x - cx) * (x - cx) + (y - cy) * (y - cy);
			if (distSq < rSq)
			{
				// The Metaball formula: intensity drops off exponentially
				float intensity = (rSq - distSq) / rSq;

				int idx = y * gridW + x;
				float current = float(maskData[idx]) / 255.0f;
				// Additive blending: this allows blobs to "merge" when they touch
				maskData[idx] = uint8_t(glm::clamp(current + intensity, 0.0f, 1.0f) * 255.0f);
			}
		}
	}
}


void AddSplat(int cx, int cy, float radius)
{
	float rSq = radius * radius;
	// Iterate only over the bounding box
	for (int y = std::max(0, int(cy - radius)); y <= std::min(gridH - 1, int(cy + radius)); ++y)
	{
		for (int x = std::max(0, int(cx - radius)); x <= std::min(gridW - 1, int(cx + radius)); ++x)
		{
			float dx = float(x) - float(cx);
			float dy = float(y) - float(cy);
			float distSq = dx * dx + dy * dy;

			if (distSq < rSq)
			{
				// Soft falloff: 1.0 at center, 0.0 at edge
				float intensity = 1.0f - (std::sqrt(distSq) / radius);

				int idx = y * gridW + x;
				float current = float(maskData[idx]) / 255.0f;

				// Additive blending allows blobs to merge (Metaball effect)
				maskData[idx] = uint8_t(glm::clamp(current + intensity, 0.0f, 1.0f) * 255.0f);
			}
		}
	}
}

void AddSplat2(int cx, int cy, float radius)
{
	auto& grid = maskData;
	for (int y = std::max(0, int(cy - radius - 2)); y <= std::min(gridH - 1, int(cy + radius + 2)); ++y)
	{
		for (int x = std::max(0, int(cx - radius - 2)); x <= std::min(gridW - 1, int(cx + radius + 2)); ++x)
		{
			float dx = float(x) - float(cx);
			float dy = float(y) - float(cy);
			float dist = std::sqrt(dx * dx + dy * dy);

			// solid interior
			if (dist < radius - 0.2f)
			{
				grid[y * gridW + x] = 255;
				continue;
			}

			// outside
			if (dist > radius + 2.0f)
				continue;

			// only edge gets noise
			float jitter = RandomRange(-2.0f, 2.0f);
			if (dist < radius + jitter)
				grid[y * gridW + x] = 255;
		}
	}
}

	/*for (int y = std::max(0, int(cy - radius)); y <= std::min(gridH - 1, int(cy + radius)); ++y)
	{
		for (int x = std::max(0, int(cx - radius)); x <= std::min(gridW - 1, int(cx + radius)); ++x)
		{
			float dx = float(x) - float(cx);
			float dy = float(y) - float(cy);
			float dist = std::sqrt(dx * dx + dy * dy);

			if (dist > radius) continue;

			float t = 1.0f - dist / radius;   
			float value = t;                  

			int idx = y * gridW + x;
			maskData[idx] = std::max(maskData[idx], uint8_t(value * 255.0f));
		}
	}*/

void DilateMask(const std::vector<uint8_t>& src, std::vector<uint8_t>& dst, int width, int height)
{
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			uint8_t m = 0;

			for (int oy = -1; oy <= 1; ++oy)
			{
				for (int ox = -1; ox <= 1; ++ox)
				{
					int sx = x + ox;
					int sy = y + oy;

					if (sx < 0 || sx >= width || sy < 0 || sy >= height)
						continue;

					m = std::max(m, src[sy * width + sx]);
				}
			}

			dst[y * width + x] = m;
		}
	}
}

DOS::DOS(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
{

	m_GroundShader = LoadProgram("ground_splat.vert", "ground_splat.frag");
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
	int count = 10;
	for (int i = 0; i < count; ++i)
	{
		// 1. Calculate the angle in radians
		// 2 * PI / count gives the step for each item
		float angle = (2.0f * glm::pi<float>() * i) / (float)count;

		// 2. Calculate X and Z positions (assuming Y is up)
		float x = std::cos(angle) * 4;
		float z = std::sin(angle) * 4;
		float y = -2.0f; // Keep it on the ground plane

		// 3. Create the translation matrix
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

		auto randScale = RandomRange(0.3, 1);
		randScale = 1;
		model = glm::scale(model,glm::vec3(randScale));

		fireWorlds.push_back(model);
	}

	
	m_FramePerParticle = std::vector<Particle>(fireWorlds.size(), { {0,1000,0,0}, });

	m_ParticleHandle = CreateBuffer(Memory{ reinterpret_cast<uint8_t*>(m_FramePerParticle.data()), m_FramePerParticle.size() * sizeof(m_FramePerParticle[0]) },ResourceUsage::StorageRead | ResourceUsage::StorageWrite | ResourceUsage::Readback);
#endif


	m_IndexHandle = CreateIndexBuffer
	(
		Memory{ reinterpret_cast<uint8_t*>(m_Indicies.data()), m_Indicies.size() * sizeof(m_Indicies[0]) }
	);
	maskData = std::vector<uint8_t>(gridH * gridW);
	blurredMaskData.resize(maskData.size());

	m_SurfaceAttachment = CreateAttachment(AttachmentType::COLOR,TextureFormat::R8_UNORM,gridW,gridH,SampleCount::e1,
		AttachmentHint::ResolveDst | AttachmentHint::ResolveSrc
		| AttachmentHint::SampledLater);
	m_SurfaceMask = CreateFramebuffer({m_SurfaceAttachment});
	//m_MorganaTex = LoadTexture("morgana.png");
#if FLIPBOOK 
	m_FireTexture = LoadTexture("fire/fire_dos2.png");
#endif
	m_StoneTexture = LoadTexture("Morgana.png");
	NoiseTexture = LoadTexture("dos_2_noise.png");

	GetCamera()->SetPosition({ 17,-14,17 });
	GetCamera()->SetCameraControl(CameraControlMode::DIRECT_CONTROL);
	GetCamera()->LookAt({ 0,0,0 });
	ExecuteFrame(0,true);

	

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
	int pixelX = static_cast<int>(u * (gridW - 1));
	int pixelY = static_cast<int>(v * (gridH- 1));

	//worldHit =  GetCamera()->GetPosition();
	//std::cout << pixelX << " " << pixelY << "\n";
	//std::cout << hit << std::endl;

	if (hit && Input::IsMousePressed(VS_MOUSE_LEFT))
	//if (hit && Input::IsMouseClicked(VS_MOUSE_LEFT))
	{
		int numSplats =1; // Number of "fire particles" in one click
		float baseRadius = 10.0f;

		for (int i = 0; i < numSplats; ++i)
		{
			// Give each splat a slightly different center and size
			float jitteredRadius = baseRadius * RandomRange(0.5f, 1.2f);
			int jitterX = pixelX + RandomRangeInt(-8, 8);
			int jitterY = pixelY + RandomRangeInt(-8, 8);
			AddSplat2(pixelX, pixelY, jitteredRadius);
			//if (RandomRangeInt(-1, 1) > 0)
			//{
			//}
			//else AddSplat(pixelX, pixelY, jitteredRadius);
		}
		//float radius = 10;
		//float jitteredRadius = radius * RandomRange(0.25f, 1.15f);
		//int jitterX = pixelX + RandomRangeInt(-2, 2);
		//int jitterY = pixelY + RandomRangeInt(-2, 2);
		//AddSplat(jitterX, jitterY, jitteredRadius);
	}

	auto texture = GetColorTexture(m_SurfaceMask);
	UpdateTexture(texture, maskData.data(), sizeof(maskData[0])*maskData.size());

#if GROUND
	SetViewRect(GrondPass, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	SetViewTransform(GrondPass, GetCamera()->GetView(), GetCamera()->GetProj());
	

	SetTransform(world);
	//SetTransform(glm::translate(world, glm::vec3(10,0,0)));
	BindTexture("u_Texture", m_StoneTexture);
	BindTexture("u_Grid", texture);
	BindTexture("u_Noise", NoiseTexture);
	BindVertexBuffer(0, m_VertexHandle);
	BindIndexBuffer(m_IndexHandle);
	Submit(GrondPass, m_GroundShader, 1);

	int screenWidth = Application::GetScreenWidth();
	int screenHeight = Application::GetScreenHeight();
	int width = 500;
	int height = 400;
	SetClipRect(screenWidth - width, screenHeight - height, width, height);
	SetRenderMode(RenderMode::SCREEN);
	BindAttachmentAsTexture("u_Scene", texture);
	BindTexture("u_Noise", NoiseTexture);
	Submit(GrondPass, m_DebugShader, 1);
#endif

#if FLIPBOOK
	SetViewRect(GrondPass, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	SetViewTransform(GrondPass, GetCamera()->GetView(), GetCamera()->GetProj());
	m_MappedPtr = static_cast<Particle*>(ReadMappedPtr(ResourceType::StorageBuffer, m_ParticleHandle.idx));
	SetDepthWrite(false);
	age += deltaTime;


	glm::vec3 camPos = GetCamera()->GetPosition();

	std::sort(fireWorlds.begin(), fireWorlds.end(), [&](const glm::mat4& a, const glm::mat4& b) {
		// Get world positions (column 3)
		glm::vec3 posA = glm::vec3(a[3]);
		glm::vec3 posB = glm::vec3(b[3]);

		// Sort by squared distance (farthest first)
		return glm::distance(camPos, posA) > glm::distance(camPos, posB);
		});

	for (auto i = 0; i < fireWorlds.size(); i++)
	{
		(m_MappedPtr+i)->lifetime.x = age*1000;

		
		SetTransform(fireWorlds[i]);
	}

	BindVertexBuffer(0, m_VertexHandle);
	BindIndexBuffer(m_IndexHandle);
	
	BindBuffer("particles", m_ParticleHandle);
	BindTexture("u_Texture", m_FireTexture);
	
	Submit(GrondPass, m_DefaultShader, fireWorlds.size());
	frame++;
	frame = frame % 255;
#endif
	ExecuteFrame(deltaTime);

}
