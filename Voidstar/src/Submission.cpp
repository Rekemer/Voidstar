#include "Prereq.h"
#include "Submission.h"
#include "Rendering/RenderContext.h"

namespace Voidstar
{
	UPtr<Submission> g_Submission = CreateUPtr<Submission>();

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry) { return {}; }

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment)
	{ 
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateShader);
		cmd.Write(vertex);

		cmd.ReadByte();
		auto str = cmd.ReadString();
		std::cout << str;
		g_Submission->GetCommandBuffer(ResourceCommand::CreateShader).Write(fragment);

		g_Submission->GetCommandBuffer(ResourceCommand::CreateProgram);
		
		return {}; 
	}
	
	ProgramHandle LoadProgram(std::string_view shader, ShaderType type) { return {}; }
	
	ShaderHandle LoadShader(std::string_view shader, ShaderType type)
	{

		return {};
	}

	void Submit(PassID id, ProgramHandle)
	{

	}

	void SubmitInit()
	{
		g_Submission->Submit = g_Submission->Render = &g_Submission->Frames[0];
	}

	void Render()
	{

	}
}



