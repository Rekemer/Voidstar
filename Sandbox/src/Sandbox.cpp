#include "Voidstar.h"
#include "Vertex.h"
using namespace Voidstar;

class Sandbox : public Voidstar::Application
{
public:
	Sandbox(std::string appName, size_t screenWidth, size_t screenHeight) : Voidstar::Application(appName, screenWidth, screenHeight)
	{
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

		GetCamera()->SetCameraControl(CameraControlMode::ROUND_CONTROL);
		GetCamera()->LookAt({ 0,0,0 });
		ExecuteFrame(0);
	}

	void Update(float deltaTime)
	{
		SetViewRect(0, 0, 0, Application::GetScreenWidth(), Application::GetScreenHeight());
		SetViewTransform(0, GetCamera()->GetView(), GetCamera()->GetProj());
		BindVertexBuffer(0, m_VertexHandle);
		BindIndexBuffer(m_IndexHandle);
		BindTexture("u_Texture",m_MorganaTex);
		Submit(0, m_DefaultShader);

		ExecuteFrame(deltaTime);

	}

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