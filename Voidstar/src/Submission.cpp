#include "Prereq.h"
#include "Submission.h"
#include "SparseSet.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"




namespace Voidstar
{
	UPtr<Submission> g_Submission = CreateUPtr<Submission>();
	SparseSet g_ShaderHandleAllocator;
	SparseSet g_ProgramHandleAllocator;



	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry) { return {}; }

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment)
	{ 
		LoadShader(vertex, ShaderType::VERTEX);
		LoadShader(vertex, ShaderType::FRAGMENT);

		g_Submission->GetCommandBuffer(ResourceCommand::CreateProgram);
		
		auto programHandle =g_ProgramHandleAllocator.GetId();
		return ProgramHandle{ programHandle };
	}
	
	ProgramHandle LoadProgram(std::string_view shader, ShaderType type) { return {}; }
	

	ShaderHandle LoadShader(std::string_view shader, ShaderType type)
	{
		auto handle = g_ShaderHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateShader);

		cmd.WriteByte(static_cast<uint8_t>(handle & 0x00FF));
		cmd.WriteByte(static_cast<uint8_t>(handle >> 8));

		cmd.WriteString(shader);
		return ShaderHandle{ handle };
	}

	void Submit(PassID id, ProgramHandle)
	{

	}

	void SetWindow(SPtr<Window> window)
	{
		g_Submission->Window = window;
	}

	void SubmitInit(InitParams init)
	{
		g_Submission->Submit = g_Submission->Render = &g_Submission->Frames[0];
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::RendererInit);
		cmd.WriteObject(init);
		//cmd.ReadByte();
		//cmd.ReadObject<InitParams>();
	}

	

	void ExecuteCommands(ResourceCommandBuffer& commandBuffer)
	{

		while (commandBuffer.IsReadable())
		{
			auto command = static_cast<ResourceCommand>(commandBuffer.ReadByte());

			switch (command)
			{
			case Voidstar::ResourceCommand::RendererInit:
			{
				auto init = commandBuffer.ReadObject<InitParams>();
				Renderer::Instance()->Init(init.width, init.height, g_Submission->Window);
				break;
			}
			case Voidstar::ResourceCommand::RendererShutdownBegin:
				break;
			case Voidstar::ResourceCommand::CreateVertexLayout:
				break;
			case Voidstar::ResourceCommand::CreateIndexBuffer:
				break;
			case Voidstar::ResourceCommand::CreateVertexBuffer:
				break;
			case Voidstar::ResourceCommand::CreateDynamicIndexBuffer:
				break;
			case Voidstar::ResourceCommand::UpdateDynamicIndexBuffer:
				break;
			case Voidstar::ResourceCommand::CreateDynamicVertexBuffer:
				break;
			case Voidstar::ResourceCommand::UpdateDynamicVertexBuffer:
				break;
			case Voidstar::ResourceCommand::CreateShader:
			{
				auto handle = commandBuffer.Read<uint16_t>();

				auto path = commandBuffer.ReadString();
				Renderer::Instance()->CompileShader(path,ShaderType::VERTEX);
				//cmd.WriteString(shader);
				break;
			}
			case Voidstar::ResourceCommand::CreateProgram:
			{

				break;
			}
			case Voidstar::ResourceCommand::CreateTexture:
				break;
			case Voidstar::ResourceCommand::UpdateTexture:
				break;
			case Voidstar::ResourceCommand::ResizeTexture:
				break;
			case Voidstar::ResourceCommand::CreateFrameBuffer:
				break;
			case Voidstar::ResourceCommand::CreateUniform:
				break;
			case Voidstar::ResourceCommand::UpdateViewName:
				break;
			case Voidstar::ResourceCommand::SetName:
				break;
			case Voidstar::ResourceCommand::End:
				break;
			case Voidstar::ResourceCommand::RendererShutdownEnd:
				break;
			case Voidstar::ResourceCommand::DestroyVertexLayout:
				break;
			case Voidstar::ResourceCommand::DestroyIndexBuffer:
				break;
			case Voidstar::ResourceCommand::DestroyVertexBuffer:
				break;
			case Voidstar::ResourceCommand::DestroyDynamicIndexBuffer:
				break;
			case Voidstar::ResourceCommand::DestroyDynamicVertexBuffer:
				break;
			case Voidstar::ResourceCommand::DestroyShader:
				break;
			case Voidstar::ResourceCommand::DestroyProgram:
				break;
			case Voidstar::ResourceCommand::DestroyTexture:
				break;
			case Voidstar::ResourceCommand::DestroyFrameBuffer:
				break;
			case Voidstar::ResourceCommand::DestroyUniform:
				break;
			case Voidstar::ResourceCommand::ReadTexture:
				break;
			default:
				break;
			}
		}
		commandBuffer.Reset();
	}

	// start calling implementation
	void Step()
	{
		std::swap(g_Submission->Submit, g_Submission->Render);
		// execute prerender commands
		ExecuteCommands(g_Submission->Render->CmdPre);
		// render commands

		// execute postrender commands
		ExecuteCommands(g_Submission->Render->CmdPost);


	}

}



