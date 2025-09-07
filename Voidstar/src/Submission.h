#pragma once
#include "ResourceCommandBuffer.h"
#include <string_view>
#include "Core.h"
#include "Rendering/ShaderType.h"
#include "InitParams.h"
#include "Window.h"
#include "glm.hpp"

namespace Voidstar
{
	VOID_HANDLE(ProgramHandle)
	VOID_HANDLE(ShaderHandle)
	VOID_HANDLE(VertexBufferHandle)
	VOID_HANDLE(FrameBufferHandle)
	


	struct Frame
	{
		// command to execute before Render/Compute API calls
		ResourceCommandBuffer CmdPre;
		// command to execute after Render/Compute API calls
		ResourceCommandBuffer CmdPost;
	};

	
	struct FramePass
	{
		


		VertexBufferHandle VertexBuffers[256];

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

		FramePass FramePasses[256];

		// am not sure how we treat it in multithreading
		SPtr<Window> Window;
		Frame  Frames[1];
		// the one registering user commands
		Frame* Submit;
		// the one doing API calls
		Frame* Render;
	};

	enum class ShaderDataType
	{
		FLOAT4,
		FLOAT3,
		FLOAT2,
		FLOAT,

	};

	struct VertexLayout
	{
		struct VertexLayoutElement
		{
			ShaderDataType type;
			size_t offset;
		};

		VertexLayout& Add(ShaderDataType type)
		{
			m_Elements.push_back(VertexLayoutElement{type,m_CurrentOffset});
			m_CurrentOffset += GetSize(type);
			return *this;
		}

	private:
		int GetSize(ShaderDataType type)
		{
			switch (type)	
			{
			case Voidstar::ShaderDataType::FLOAT4:
				return sizeof(float) * 4;
				break;
			case Voidstar::ShaderDataType::FLOAT3:
				return sizeof(float) * 3;
				break;
			case Voidstar::ShaderDataType::FLOAT2:
				return sizeof(float) * 2;
				break;
			case Voidstar::ShaderDataType::FLOAT:
				return sizeof(float);
				break;
			default:
				assert(false);
				return -1;
				break;
			}
		}
		std::vector<VertexLayoutElement> m_Elements;
		size_t m_CurrentOffset = 0;
	};


	using PassID = uint16_t;


	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment);

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry);

	ProgramHandle LoadProgram(std::string_view program, ShaderType type);

	ShaderHandle LoadShader(std::string_view shader, ShaderType type);
	void SetWindow(SPtr<Window> window);
	void SubmitInit(InitParams);
	void Submit(PassID id, ProgramHandle program);

	// shader location
	inline void SetVertexBuffer(uint16_t location, VertexBufferHandle handle){};
	
	inline VertexBufferHandle CreateVertexBuffer() { return{}; };

	void Step();

}