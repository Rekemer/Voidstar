#pragma once
#include <vector>
#include "Utils.h"
namespace Voidstar
{

	void Init();

	enum class ResourceCommand
	{
		RendererInit,
		RendererShutdownBegin,
		CreateVertexLayout,
		CreateIndexBuffer,
		CreateVertexBuffer,
		CreateDynamicIndexBuffer,
		UpdateDynamicIndexBuffer,
		CreateDynamicVertexBuffer,
		UpdateDynamicVertexBuffer,
		CreateShader,
		CreateProgram,
		CreateTexture,
		UpdateTexture,
		ResizeTexture,
		CreateFrameBuffer,
		CreateUniform,
		UpdateViewName, // ?
		InvalidateOcclusionQuery,
		SetName,
		End,
		RendererShutdownEnd,
		DestroyVertexLayout,
		DestroyIndexBuffer,
		DestroyVertexBuffer,
		DestroyDynamicIndexBuffer,
		DestroyDynamicVertexBuffer,
		DestroyShader,
		DestroyProgram,
		DestroyTexture,
		DestroyFrameBuffer,
		DestroyUniform,
		ReadTexture,
	};

	using ResourceCommandBuffer = std::vector<uint8_t>;


	


	

	void Submit(PassID id, ProgramHandle program);
	void Render();
}