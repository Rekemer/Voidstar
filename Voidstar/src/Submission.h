#pragma once
#include "ResourceCommandBuffer.h"
#include <string_view>
#include "Core.h"
#include "Rendering/ShaderType.h"

namespace Voidstar
{


	struct Frame
	{

		// command to execute before Render/Compute API calls
		ResourceCommandBuffer CmdPre;
		// command to execute after Render/Compute API calls
		ResourceCommandBuffer CmdPost;
	};

	

	struct Submission
	{

		ResourceCommandBuffer& GetCommandBuffer(ResourceCommand command)
		{
			auto& cmdBuf = command < ResourceCommand::End ? Submit->CmdPre : Submit->CmdPost;

			// alignment ???

			//template<typename Type>
			//void write(const Type & _in)
			//{
			//	align(BX_ALIGNOF(Type));
			//	write(reinterpret_cast<const uint8_t*>(&_in), sizeof(Type));
			//}

			cmdBuf.WriteByte(static_cast<uint8_t>(command));

			return cmdBuf;
		}

		Frame  Frames[1];
		// the one registering user commands
		Frame* Submit;
		// the one doing API calls
		Frame* Render;
	};

	VOID_HANDLE(ProgramHandle)
	VOID_HANDLE(ShaderHandle)

	using PassID = uint16_t;


	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment);

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry);

	ProgramHandle LoadProgram(std::string_view program, ShaderType type);

	ShaderHandle LoadShader(std::string_view shader, ShaderType type);

	void SubmitInit();
	void Submit(PassID id, ProgramHandle program);
	void Render();

}