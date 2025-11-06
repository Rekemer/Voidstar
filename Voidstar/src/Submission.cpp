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
	SparseSet<TextureHandle> g_TextureHandleAllocator;
	SparseSet<UniformHandle> g_UniformHandleAllocator;
	SparseSet<FrameBufferHandle> g_FramebufferHandleAllocator;
	SparseSet<AttachmentHandle> g_AttachmentrHandleAllocator;


	
	TextureHandle GetTextureHandle()
	{
		return g_TextureHandleAllocator.GetId();
	}
	FrameBufferHandle GetFrameBufferHandle()
	{
		return g_FramebufferHandleAllocator.GetId();
	}
	AttachmentHandle GetAttachmentHandle()
	{
		return g_AttachmentrHandleAllocator.GetId();
	}
	VertexLayout GetVertexLayout(VertexLayoutHandle handle)
	{
		return g_Submission->Layouts.at(handle);
	}

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry) { return {}; }

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment)
	{ 
		LoadShader(vertex, ShaderType::VERTEX);
		LoadShader(fragment, ShaderType::FRAGMENT);

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

		cmd.WriteObject(handle);
		cmd.WriteString(shader);
		return handle;
	}


	TextureHandle LoadTexture(std::string_view texture)
	{
		auto handle = g_TextureHandleAllocator.GetId();

		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateTexture);
		cmd.WriteObject(handle);
		cmd.WriteString(texture);

		return handle;

	}


	UniformHandle CreateUniform(std::string_view name, ResourceType kind, size_t num)
	{
		auto handle = g_UniformHandleAllocator.GetId();

		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateUniform);
		cmd.WriteObject(kind);
		cmd.WriteObject(num);

		return handle;

	}
	AttachmentHandle CreateAttachment(AttachmentType type, TextureFormat format, int width, int height, SampleCount samples, AttachmentHint hints)
	{
		auto handle = g_AttachmentrHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateAttachment);
		cmd.WriteObject(handle);
		AttachmentInfo_ info{ type,format,width,height,samples,hints };
		cmd.WriteObject(info);
		return handle;
	}

	void BindAttachmentAsTexture(std::string_view name, TextureHandle handle)
	{
		auto& bind = g_Submission->Submit->CurrentRenderItem->ResBindings[g_Submission->Submit->CurrentRenderItem->currentResBinding++];

		bind.uniform = name;
		bind.dirty = true;
		bind.handles[bind.currentHandle++] = handle;
	}
	TextureHandle GetColorTexture(FrameBufferHandle fb)
	{
		auto handle = Renderer::Instance()->GetFBTextureHandle(fb);
		if (!handle.Valid()) assert(false);
		return handle;
	}

	FrameBufferHandle CreateFramebuffer(const std::vector<AttachmentHandle>& handles)
	{
		auto handle = g_FramebufferHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateFrameBuffer);
		cmd.WriteObject(handle);
		cmd.WriteObject(handles.size());
		for (auto h : handles)
		{
			cmd.WriteObject(h);
		}
		return handle;
	}


	void BindTextures(std::string_view uniformName, const std::vector<TextureHandle>& handles)
	{
		assert(false);
		/*auto& bind = g_Submission->Submit->CurrentRenderItem->ResBindings[g_Submission->Submit->CurrentRenderItem->currentResBinding++];

		bind.uniform = uniformName;
		bind.dirty = true;
		bind.handles = handles;*/
	}

	void BindTexture(std::string_view uniformName, TextureHandle handle)
	{
		auto& bind = g_Submission->Submit->CurrentRenderItem->ResBindings[g_Submission->Submit->CurrentRenderItem->currentResBinding++];

		bind.uniform = uniformName;
		bind.dirty = true;
		bind.handles[bind.currentHandle++] = handle;
	}
	void SetViewTransform(PassID id, glm::mat4& view, glm::mat4& proj)
	{
		g_Submission->Submit->Views[id].View = view;
		g_Submission->Submit->Views[id].Proj= proj;
	}
	void SetFramebuffer(PassID id ,FrameBufferHandle handle)
	{
		g_Submission->Submit->Views[id].Fbh= handle;
	}
	void SetViewRect(PassID id, size_t x, size_t y, size_t width, size_t height)
	{
		g_Submission->Submit->Views[id].Rect = { x,y,width,height };
	}

	void Submit(PassID viewID, ProgramHandle programHandle)
	{
		// creates render item
		auto renderItem = g_Submission->Submit->CurrentRenderItem;
		renderItem->Program = programHandle;
		renderItem->View =viewID;
		// we can create pipeline
		g_Submission->Submit->NextItem();
		
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

				Renderer::Instance()->Init(init.width, init.height, g_Submission->Window,init.app);
				
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
			{

				auto handle = commandBuffer.ReadObject<TextureHandle>();
				auto path = commandBuffer.ReadString();
				Renderer::Instance()->CreateTexture(handle, path);
				break;
			}
			case Voidstar::ResourceCommand::UpdateTexture:
				break;
			case Voidstar::ResourceCommand::ResizeTexture:
				break;
			case Voidstar::ResourceCommand::CreateAttachment:
			{
				auto handle = commandBuffer.ReadObject<AttachmentHandle>();
				auto info = commandBuffer.ReadObject<AttachmentInfo_>();
				Renderer::Instance()->CreateAttachment(handle,info);
				break;
			}
			case Voidstar::ResourceCommand::CreateFrameBuffer:
			{
				auto handle = commandBuffer.ReadObject<FrameBufferHandle>();
				auto amount= commandBuffer.ReadObject<size_t>();
				std::vector<AttachmentHandle> handles;
				handles.reserve(amount);
				for (int i = 0; i < amount; i++)
				{
					handles.push_back(commandBuffer.ReadObject<AttachmentHandle>());
				}
				Renderer::Instance()->CreateFramebuffer(handle,handles);
				break;
			}
			case Voidstar::ResourceCommand::CreateUniform:
			{

				auto handle = commandBuffer.ReadObject<UniformHandle>();
				auto type = commandBuffer.ReadObject<ResourceType>();
				auto num = commandBuffer.Read<size_t>();
				Renderer::Instance()->CreateUniform(handle, type, num);
				break;
			}
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
	void ExecuteFrame(float deltaTime)
	{
		std::swap(g_Submission->Submit, g_Submission->Render);
		// execute prerender commands
		ExecuteCommands(g_Submission->Render->CmdPre);
		// render commands

		Renderer::Instance()->RenderFrame(g_Submission->Render, deltaTime);

		// execute postrender commands
		ExecuteCommands(g_Submission->Render->CmdPost);
		g_Submission->Render->Reset();
	}
	void BindIndexBuffer(IndexBufferHandle handle)
	{
		g_Submission->Submit->CurrentRenderItem->IndexBuffer = handle;
	};
	void BindVertexBuffer(uint16_t location, VertexBufferHandle handle)
	{
		g_Submission->Submit->CurrentRenderItem->Bindings[location].VertexHandle = handle;
		g_Submission->Submit->CurrentRenderItem->Bindings[location].LayoutHandle = g_Submission->VertexLayoutMap.at(handle);
		g_Submission->Submit->CurrentRenderItem->currentBinding++;
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



