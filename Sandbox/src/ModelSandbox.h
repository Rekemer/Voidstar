#pragma once
#include "Voidstar.h"
#include "Vertex.h"
using namespace Voidstar;

class ModelSandbox : public Voidstar::Application
{
public:
	ModelSandbox(std::string appName, size_t screenWidth, size_t screenHeight);
	void Update(float deltaTime) override;
private:
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
