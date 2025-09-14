#include "Prereq.h"
#include "Submission.h"
#include "SparseSet.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"




namespace Voidstar
{
	UPtr<Submission> g_Submission = CreateUPtr<Submission>();

	SparseSet<ShaderHandle> g_ShaderHandleAllocator;
	SparseSet<ProgramHandle> g_ProgramHandleAllocator;

	SparseSet<VertexBufferHandle> g_VertexBufferHandleAllocator;
	SparseSet<IndexBufferHandle> g_IndexBufferHandleAllocator;
	SparseSet<VertexLayoutHandle> g_LayoutHandleAllocator;



	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry) { return {}; }

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment)
	{ 
		LoadShader(vertex, ShaderType::VERTEX);
		LoadShader(vertex, ShaderType::FRAGMENT);

		auto programHandle =g_ProgramHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateProgram);
		
		cmd.WriteObject(programHandle);
		cmd.WriteByte(2);

		return ProgramHandle{ programHandle };
	}
	
	ProgramHandle LoadProgram(std::string_view shader, ShaderType type) { return {}; }
	

	ShaderHandle LoadShader(std::string_view shader, ShaderType type)
	{
		auto handle = g_ShaderHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateShader);

		cmd.WriteByte(static_cast<uint8_t>(handle.idx & 0x00FF));
		cmd.WriteByte(static_cast<uint8_t>(handle.idx >> 8));

		cmd.WriteString(shader);
		return ShaderHandle{ handle };
	}

	void SetViewTransform(PassID id, glm::mat4& view, glm::mat4& proj)
	{
		g_Submission->Views[id].View = view;
		g_Submission->Views[id].Proj= proj;
	}
	void SetViewRect(PassID id, size_t x, size_t y, size_t width, size_t height)
	{
		g_Submission->Views[id].Rect = { x,y,width,height };
	}

	void Submit(PassID viewID, ProgramHandle programHandle)
	{
		// creates render item
		auto& renderItem = g_Submission->CurrentRenderItem;
		renderItem->Program = programHandle;
		renderItem->View =viewID;
		// we can create pipeline

	
		g_Submission->NextItem();
		
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
			{
				auto mem = commandBuffer.ReadObject<Memory>();
				auto bufferHandle = commandBuffer.ReadObject<uint16_t>();
				Renderer::Instance()->CreateIndexBuffer(mem, IndexBufferHandle{ bufferHandle });
			}
				break;
			case Voidstar::ResourceCommand::CreateVertexBuffer:
			{
				auto mem = commandBuffer.ReadObject<Memory>();
				auto bufferHandle = commandBuffer.ReadObject<uint16_t>();
				auto layoutHandle = commandBuffer.ReadObject<uint16_t>();
				Renderer::Instance()->CreateVertexBuffer(mem, VertexBufferHandle{ bufferHandle });
			}
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
				Renderer::Instance()->CompileShader(path);
				break;
			}
			case Voidstar::ResourceCommand::CreateProgram:
			{
				auto handle = commandBuffer.Read<uint16_t>();
				auto shaderAmount = commandBuffer.ReadByte();
				Renderer::Instance()->LinkShaders(ProgramHandle{ handle }, shaderAmount);
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
	void ExecuteFrame()
	{
		std::swap(g_Submission->Submit, g_Submission->Render);
		// execute prerender commands
		ExecuteCommands(g_Submission->Render->CmdPre);
		// render commands

		// execute postrender commands
		ExecuteCommands(g_Submission->Render->CmdPost);


	}
	void BindIndexBuffer(IndexBufferHandle handle)
	{
		g_Submission->CurrentRenderItem->IndexBuffer = handle;
	};
	void BindVertexBuffer(uint16_t location, VertexBufferHandle handle)
	{
		g_Submission->CurrentRenderItem->Bindings[location].VertexHandle = handle;
		g_Submission->CurrentRenderItem->Bindings[location].LayoutHandle = g_Submission->VertexLayoutMap.at(handle);
	};


	VertexBufferHandle CreateVertexBuffer(Memory mem, VertexLayout& layout)
	{
		auto bufferHandle = VertexBufferHandle{ g_VertexBufferHandleAllocator.GetId() };
		

		auto layoutHandle = VertexLayoutHandle{g_LayoutHandleAllocator.GetId()};
		g_Submission->Layouts.insert({layoutHandle,layout});
		g_Submission->VertexLayoutMap.insert({ bufferHandle, layoutHandle });

		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateVertexBuffer);
		cmd.WriteObject(mem);
		//auto me = cmd.ReadObject<Memory>();
		cmd.WriteObject(bufferHandle);
		cmd.WriteObject(layoutHandle);

		return{ bufferHandle };
	};
	IndexBufferHandle CreateIndexBuffer(Memory mem)
	{
		auto bufferHandle = g_IndexBufferHandleAllocator.GetId();

		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateIndexBuffer);

		cmd.WriteObject(mem);
		//auto me = cmd.ReadObject<Memory>();
		cmd.WriteObject(bufferHandle);
		return { bufferHandle };
	}

}



