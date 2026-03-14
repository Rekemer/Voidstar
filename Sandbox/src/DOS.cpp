#include "DOS.h"
#include <random>

struct Particle
{
	glm::vec4 lifetime;
	glm::mat4 world;    	
	float distanceSq;   
};
VertexBufferHandle m_VertexHandle;
BufferHandle m_InstanceHandle;
IndexBufferHandle m_IndexHandle;
ProgramHandle m_DefaultShader;
ProgramHandle m_GroundShader;
ProgramHandle m_DebugShader;
ProgramHandle m_DownSamplingShader;
ProgramHandle m_UpsamplingShader;
std::vector<Vertex> m_Verticies;
std::vector<glm::vec4> m_FramePerParticle;
std::vector<IndexType> m_Indicies;
VertexLayout m_VertexLayout;
VertexLayout m_InstanceLayout;
TextureHandle m_MorganaTex;
TextureHandle NoiseTexture;
TextureHandle NoiseTexture1;
TextureHandle NoiseTexture2;
TextureHandle NoiseTexture3;
BufferHandle m_ParticleHandle;
TextureHandle m_FireTexture;
TextureHandle m_StoneTexture;


FrameBufferHandle m_SurfaceMask;
AttachmentHandle m_SurfaceAttachment;

FrameBufferHandle downsampleFramebuffer;
AttachmentHandle bloomAttachments[3];
FrameBufferHandle upsampleFramebuffer;


glm::vec4* m_MappedPtr;
glm::mat4 worldGround;
PassID GrondPass = 0;
PassID Flipbook = 1;
#define FLIPBOOK 0
#define BLOOM 0
#define GROUND 1
#define DEBUG 1

struct BloomLevel
{
	int w;
	int h;
	AttachmentHandle attachment;
	FrameBufferHandle fbh;
};

std::vector<uint8_t> maskData;
std::vector<glm::vec2> clickedPixels;
std::vector<uint8_t> blurredMaskData;
std::vector<Particle> fireWorlds;


float planeSize = 10;
float planeHalfSize = planeSize/2;
int clicked = 0;
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

glm::vec3 PixelToWorldOnPlane(int x, int y, glm::mat4& world)
{
	float u = (x + 0.5f) / float(gridW);
	float v = (y + 0.5f) / float(gridH);
	float localX = u * 2.0f - 1.0;
	float localY = v * 2.0f - 1.0f;

	glm::vec3 local(localX, localY, 0.0f);

	return glm::vec3(world * glm::vec4(local, 1.0f));


}
void SpawnParticle(int index, glm::vec3 pos)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
	auto randScale = RandomRange(0.6f, 1.2f);
	model = glm::scale(model, glm::vec3(randScale));
	Particle p;
	p.world = model;
	p.lifetime = { 0,1000,0,0 };
	p.distanceSq = 0;
	fireWorlds.push_back(p);
}

void AddZone(int pixelX, int pixelY)
{
	// Inside your Update() function when hit && Input::IsMousePressed
	float minSeparation = RandomRange(40.0f, 60.0f)+ RandomRange(-5.0f, 5.0f);
	bool tooClose = false;

	for (const auto& zone : clickedPixels) {
		float dx = (float)pixelX - (float)zone.x;
		float dy = (float)pixelY - (float)zone.y;
		float distSq = dx * dx + dy * dy;

		if (distSq < (minSeparation * minSeparation)) {
			tooClose = true;
			// Optional: Refresh the strength of the existing zone so it doesn't die
			// zone.strength = 1.0f; 
			break;
		}
	}

	if (!tooClose) {
		// Only add a new spawning source if we aren't standing on an old one
		clickedPixels.push_back({ pixelX, pixelY });

		glm::vec3 spawnPos = PixelToWorldOnPlane(pixelX, pixelY, worldGround);
		// Jitter the position so they don't look like they are in a grid
		spawnPos.x += RandomRange(-0.2f, 0.2f);
		spawnPos.z += RandomRange(-0.2f, 0.2f);
		spawnPos.y -= 1 ;
		clicked++;
		SpawnParticle(clicked - 1, spawnPos);

	}
}

void AddInverseSplat(int cx, int cy, float radius)
{
	float rSq = radius * radius;
	AddZone(cx, cy);

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
	AddZone(cx, cy);
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
	AddZone(cx,cy);
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
			auto idx = y * gridW + x;
			if (dist < radius + jitter)
			{
				grid[idx] = 255;
			}
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

std::vector<BloomLevel> m_DownsampleChain;
std::vector<PassID> m_DownsamplePasses;
std::vector<PassID> m_UpsamplePasses;
int bloomPasses = 6;
DOS::DOS(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
{

	m_GroundShader = LoadProgram("ground_splat.vert", "bloom_test.frag");
	m_DebugShader = LoadProgram("screen.vert", "render_attachment.frag");
	m_DownSamplingShader = LoadProgram("render_screen_quad.vert", "downsample.frag");
	m_UpsamplingShader = LoadProgram("render_screen_quad.vert", "upsample.frag");
	m_VertexLayout.AddVertex(ShaderDataType::FLOAT3, 0)
		.AddVertex(ShaderDataType::FLOAT2, 0);
	
	auto [verts, indices] = GeneratePlane<Vertex>(2);
	m_Verticies = verts;
	m_Indicies = indices;
	m_VertexHandle = CreateVertexBuffer({
		reinterpret_cast<uint8_t*>(m_Verticies.data()),m_Verticies.size() * sizeof(m_Verticies[0]) }
	, m_VertexLayout);

	bloomAttachments[0] = CreateAttachment(AttachmentType::COLOR,TextureFormat::RGBA16_SFLOAT, 
		Application::GetScreenWidth(), Application::GetScreenHeight(),SampleCount::e1,AttachmentHint::SampledLater);
	bloomAttachments[1] =  CreateAttachment(AttachmentType::COLOR,TextureFormat::RGBA16_SFLOAT,
 Application::GetScreenWidth(), Application::GetScreenHeight(),SampleCount::e1,AttachmentHint::SampledLater);
	bloomAttachments[2] = CreateAttachment(AttachmentType::COLOR, TextureFormat::RGBA16_SFLOAT,
 Application::GetScreenWidth(), Application::GetScreenHeight(), SampleCount::e1, AttachmentHint::None);
	downsampleFramebuffer = CreateFramebuffer({ bloomAttachments[0],bloomAttachments[1]});
	upsampleFramebuffer= CreateFramebuffer({ bloomAttachments[2]});



	uint32_t w = Application::GetScreenWidth() / 2;
	uint32_t h = Application::GetScreenHeight() / 2;

	for (int i = 1; i < bloomPasses+1; i++)
	{
		m_DownsamplePasses.push_back(Flipbook+i);
	}
	for (int i = 1; i < bloomPasses + 1; i++)
	{
		m_UpsamplePasses.push_back(m_DownsamplePasses.back() + i);
	}

	for (int i = 0; i < bloomPasses; i++) { // 6 levels of blur
		BloomLevel level;
		level.w= w;
		level.h= h;

		// Create attachment at this specific size
		level.attachment= CreateAttachment(AttachmentType::COLOR, TextureFormat::RGBA16_SFLOAT, w, h, SampleCount::e1, 
			AttachmentHint::SampledLater);

		// Create a framebuffer specifically for this size
		level.fbh = CreateFramebuffer({ level.attachment });

		m_DownsampleChain.push_back(level);

		// Shrink for next level
		w = std::max(1u, w / 2);
		h = std::max(1u, h / 2);
	}



#if FLIPBOOK
	m_DefaultShader = LoadProgram("flipbook.vert", "flipbook.frag");
	int count = 100;

	m_FramePerParticle = std::vector<glm::vec4>(100,  {0,1000,0,0});

	for (auto& p : m_FramePerParticle)
	{
		p.x = RandomRange(0.0f, p.y);
	}


	m_ParticleHandle = CreateBuffer(Memory{ reinterpret_cast<uint8_t*>(m_FramePerParticle.data()), m_FramePerParticle.size() * sizeof(m_FramePerParticle[0]) },
		ResourceUsage::StorageRead | ResourceUsage::StorageWrite | ResourceUsage::Readback);
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
	NoiseTexture1 = LoadTexture("dos_2_noise_1.png");
	NoiseTexture2 = LoadTexture("dos_2_noise_2.png");
	NoiseTexture3 = LoadTexture("dos_2_noise_3.png");

	GetCamera()->SetPosition({ 17,-14,17 });
	GetCamera()->SetCameraControl(CameraControlMode::DIRECT_CONTROL);
	GetCamera()->LookAt({ 0,0,0 });
	ExecuteFrame(0,true);

	worldGround = glm::mat4(1);
	worldGround = glm::rotate(worldGround, glm::radians(90.f), glm::vec3(1, 0, 0));
	worldGround = glm::scale(worldGround, glm::vec3(planeSize));

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

	auto screenWidth = Application::GetScreenWidth();
	auto screenHeight = Application::GetScreenHeight();

	Ray ray = ScreenPointToRay(std::get<0>(mousePos), std::get<1>(mousePos), screenWidth, screenHeight, GetCamera()->GetView(), GetCamera()->GetProj());

	glm::vec3 localHit;
	bool hit = IntersectPlane(
		ray,
		worldGround,
		localHit
	);
	glm::vec3 worldHit = glm::vec3(worldGround * glm::vec4(localHit,1));
	
	float u = (localHit.x + 1.0f) * 0.5f;
	float v = (localHit.y + 1.0f) * 0.5f;

	// 2. UV -> Pixel [0, maskRes]
	int pixelX = static_cast<int>(u * (gridW - 1));
	int pixelY = static_cast<int>(v * (gridH- 1));

	
	
	if (hit && Input::IsMousePressed(VS_MOUSE_LEFT))
	//if (hit && Input::IsMouseClicked(VS_MOUSE_LEFT))
	{
		int numSplats =1; // Number of "fire particles" in one click
		float baseRadius = 20.0f;
		for (int i = 0; i < numSplats; ++i)
		{
			// Give each splat a slightly different center and size
			float jitteredRadius = baseRadius * RandomRange(0.5f, 1.2f);
			int jitterX = pixelX + RandomRangeInt(-8, 8);
			int jitterY = pixelY + RandomRangeInt(-8, 8);
			if (RandomRangeInt(-1, 1) > 0)
			{
				AddInverseSplat(pixelX, pixelY, jitteredRadius);
			}
			else AddSplat(pixelX, pixelY, jitteredRadius);

		}
		
	}

	auto texture = GetColorTexture(m_SurfaceMask);
	UpdateTexture(texture, maskData.data(), sizeof(maskData[0])*maskData.size());

#if GROUND
	SetViewRect(GrondPass, 0, 0, screenWidth,screenHeight);
	SetViewTransform(GrondPass, GetCamera()->GetView(), GetCamera()->GetProj());
	
	
	SetFramebuffer(GrondPass,downsampleFramebuffer);
	BlendMode state;
	state.enabled = false;
	SetBlendState(0, state);
	SetBlendState(1, state);
	
	SetTransform(worldGround);
	//SetTransform(glm::translate(world, glm::vec3(10,0,0)));
	//BindTexture("u_Texture", m_StoneTexture);
	//BindTexture("u_Grid", texture);
	//BindTextures("u_Noise", { NoiseTexture,NoiseTexture1,NoiseTexture2,NoiseTexture3 });
	BindVertexBuffer(0, m_VertexHandle);
	BindIndexBuffer(m_IndexHandle);
	Submit(GrondPass, m_GroundShader, 1);


	SetViewRect(m_DownsamplePasses[0], 0, 0, screenWidth / 2, screenHeight / 2);

	SetFramebuffer(m_DownsamplePasses[0], m_DownsampleChain[0].fbh);
	BlendMode mode;
	mode.enabled = false;
	SetBlendState(0, mode);


	auto attachment = GetColorTexture(downsampleFramebuffer,1);

	BindAttachmentAsTexture("u_Source", attachment);

	Submit(m_DownsamplePasses[0], m_DownSamplingShader, 1);

	std::cout << "BEGIN\n";
	for (auto i =0; i < m_DownsampleChain.size() - 1; i++)
	{
		auto& level = m_DownsampleChain[i];
		PassID pass = m_DownsamplePasses[i+1];
		auto& destLevel = m_DownsampleChain[i + 1];
		SetViewRect(pass, 0, 0, destLevel.w, destLevel.h);
		auto tex = GetColorTexture(level.fbh);
		std::cout << tex.idx << "\n";
		BindAttachmentAsTexture("u_Source", tex);
		BlendMode mode;
		mode.enabled = false;
		SetBlendState(0, mode);
		// DESTINATION is the next FB in the chain
		SetFramebuffer(pass, m_DownsampleChain[i + 1].fbh);

		Submit(pass, m_DownSamplingShader, 1);
	}
	std::cout << "END\n";

	// Start from the second-to-last level
	for (int i = m_DownsampleChain.size() - 1; i > 0; i--)
	{
		auto& smallLevel = m_DownsampleChain[i];     
		auto& bigLevel = m_DownsampleChain[i - 1];   

		PassID upPass = m_UpsamplePasses[i];

		SetViewRect(upPass, 0, 0, bigLevel.w, bigLevel.h);
		SetFramebuffer(upPass, bigLevel.fbh); 
		BindAttachmentAsTexture("u_Source", GetColorTexture(smallLevel.fbh));
		BlendMode mode;
		mode.enabled = false;
		mode.srcColor = BlendFactor::One;
		mode.dstColor = BlendFactor::One;
		SetBlendState(0, mode);
		Submit(upPass, m_UpsamplingShader, 1);
	}


	#if DEBUG
	SetViewRect(Flipbook, 0, 0, screenWidth, screenHeight);
	SetViewTransform(Flipbook, GetCamera()->GetView(), GetCamera()->GetProj());
		
		int width = screenWidth/4;
		int height = screenHeight/4;
		SetClipRect(screenWidth - width, 0, width, height);
		SetRenderMode(RenderMode::SCREEN);
		auto bloomTex = GetColorTexture(m_DownsampleChain[0].fbh);
		BindAttachmentAsTexture("u_Scene", bloomTex);
		BindTexture("u_Noise", NoiseTexture);
		Submit(Flipbook, m_DebugShader, 1);
	#endif
#endif

#if FLIPBOOK
	SetViewRect(GrondPass, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
	SetViewTransform(GrondPass, GetCamera()->GetView(), GetCamera()->GetProj());
	m_MappedPtr = static_cast<glm::vec4*>(ReadMappedPtr(ResourceType::StorageBuffer, m_ParticleHandle.idx));
	SetDepthWrite(false);
	

	glm::vec3 camPos = GetCamera()->GetPosition();
	for (auto& p : fireWorlds) {
		glm::vec3 pos = glm::vec3(p.world[3]); // Extract world position
		p.distanceSq = glm::distance(camPos, pos);
	}

	//std::sort(fireWorlds.begin(), fireWorlds.end(),
	//	[](const Particle& a, const Particle& b) {
	//		return a.distanceSq > b.distanceSq; // Farthest first
	//	});

	
	for (int i = 0; i < fireWorlds.size(); i++)
	{
		m_MappedPtr[i].x += deltaTime * 2200;
		SetTransform(fireWorlds[i].world);
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
