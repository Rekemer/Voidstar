#pragma once
#include "ResourceCommandBuffer.h"
#include <string_view>
#include "Core.h"
#include "Rendering/ShaderType.h"
#include "InitParams.h"
#include "Window.h"
#include "VertexLayout.h"
#include "glm.hpp"
#include <unordered_map>
#include <cstdint>

namespace Voidstar
{
	template <class Tag>
	struct Handle {
		
		static constexpr uint16_t INVALID_ID = uint16_t(-1);

		uint16_t idx = INVALID_ID;

		// validity
		bool Valid() const { return idx != INVALID_ID; }
	};


	struct ProgramTag {};
	struct ShaderTag {};
	struct VertexBufferTag {};
	struct IndexBufferTag {};
	struct FramebufferTag {};
	struct VertexLayoutTag {};

	using ProgramHandle = Handle<ProgramTag>;
	using ShaderHandle = Handle<ShaderTag>;
	using VertexBufferHandle = Handle<VertexBufferTag>;
	using IndexBufferHandle = Handle<IndexBufferTag>;
	using FrameBufferHandle = Handle<FramebufferTag>;
	using VertexLayoutHandle = Handle<VertexLayoutTag>;
	using PassID = uint16_t;
	
	
	struct Memory
	{
		uint8_t* data; //!< Pointer to data.
		size_t size; //!< Data size.
	};

	struct VertexBinding
	{
		VertexBufferHandle VertexHandle;
		VertexLayoutHandle LayoutHandle;
	};
	// render items learns about the view at submit
	struct RenderItem
	{
		PassID View;
		VertexBinding Bindings[10];
	};

	struct Frame
	{

		RenderItem m_renderItem[256];

		// command to execute before Render/Compute API calls
		ResourceCommandBuffer CmdPre;
		// command to execute after Render/Compute API calls
		ResourceCommandBuffer CmdPost;
	};
	struct View
	{
		glm::vec4 Rect;
		glm::mat4 View;
		glm::mat4 Proj;
		FrameBufferHandle Fbh;
	};
	

	struct Submission
	{

		ResourceCommandBuffer& GetCommandBuffer(ResourceCommand command)
		{
			auto& cmdBuf = command < ResourceCommand::End ? Submit->CmdPre : Submit->CmdPost;

			cmdBuf.WriteByte(static_cast<uint8_t>(command));

			return cmdBuf;
		}

		View Views[256];
		RenderItem* CurrentRenderItem = Frames[0].m_renderItem;

		std::unordered_map<uint16_t, uint16_t> VertexLayoutMap;
		std::unordered_map<uint16_t, VertexLayout> Layouts;
		// am not sure how we treat it in multithreading
		SPtr<Window> Window;
		Frame  Frames[1];
		// the one registering user commands
		Frame* Submit;
		// the one doing API calls
		Frame* Render;
	};

	

	


	


	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment);

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry);

	ProgramHandle LoadProgram(std::string_view program, ShaderType type);

	ShaderHandle LoadShader(std::string_view shader, ShaderType type);
	void SetWindow(SPtr<Window> window);
	void SubmitInit(InitParams);


	void SetViewTransform(PassID id, glm::mat4& view, glm::mat4& proj);
	void SetViewRect(PassID id , size_t x, size_t y, size_t width, size_t height);

	void Submit(PassID id, ProgramHandle program);
	void BindIndexBuffer(IndexBufferHandle handle);
	// shader location
	void BindVertexBuffer(uint16_t location , VertexBufferHandle handle);

	VertexBufferHandle CreateVertexBuffer(Memory mem, VertexLayout& layout);
	IndexBufferHandle CreateIndexBuffer(Memory mem);

	void ExecuteFrame();

}