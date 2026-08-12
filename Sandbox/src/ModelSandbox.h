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
	FontHandle m_Font;
	PassID m_DefaultRenderPass;
	VertexBufferHandle m_VertexHandle;
	IndexBufferHandle m_IndexHandle;
	ProgramHandle m_DefaultShader;
	ProgramHandle m_FontShader;
	ProgramHandle m_CompositeShader;
	
	FrameBufferHandle m_UIFrameBuffer;

	std::vector<Vertex> m_Cube;
	std::vector<IndexType> m_IndexCube;


	std::vector<Vertex_> m_Quad;
	VertexBufferHandle m_VertexQuadBuffer;
	std::vector<IndexType> m_IndexQuad;
	IndexBufferHandle m_IndexQuadBuffer;
	
	VertexLayout m_VertexLayout;
	VertexLayout m_VertexLayoutQuad;
	TextureHandle m_MorganaTex;
};
