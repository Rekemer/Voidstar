#include "Prereq.h"
#include "Submission.h"
#include "SparseSet.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include <semaphore>
#include <print>


#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <filesystem>


// for loading image from gltf files
#include "stb_image.h"

namespace Voidstar
{
	UPtr<Submission> g_Submission = CreateUPtr<Submission>();

	SparseSet<ShaderHandle> g_ShaderHandleAllocator;
	SparseSet<ProgramHandle> g_ProgramHandleAllocator;

	SparseSet<BufferHandle> g_BufferHandleAllocator;
	SparseSet<VertexBufferHandle> g_VertexBufferHandleAllocator;
	SparseSet<IndexBufferHandle> g_IndexBufferHandleAllocator;
	SparseSet<VertexLayoutHandle> g_LayoutHandleAllocator;
	SparseSet<TextureHandle> g_TextureHandleAllocator;
	SparseSet<UniformHandle> g_UniformHandleAllocator;
	SparseSet<FrameBufferHandle> g_FramebufferHandleAllocator;
	SparseSet<AttachmentHandle> g_AttachmentrHandleAllocator;

	struct UpdateImageRegion
	{
		Memory mem;
		size_t width;
		size_t height;
		TextureHandle imageToUpdate;
		glm::vec3 offset;
		int layer;
	};





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


	ProgramHandle LoadComputeProgram(std::string_view cmp)
	{
		LoadShader(cmp);
		auto programHandle = g_ProgramHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateProgram);

		cmd.WriteObject(programHandle);
		cmd.WriteByte(1);

		return programHandle;
	}

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment)
	{
		LoadShader(vertex);
		LoadShader(fragment);

		auto programHandle = g_ProgramHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateProgram);

		cmd.WriteObject(programHandle);
		cmd.WriteByte(2);

		return programHandle;
	}



	ShaderHandle LoadShader(std::string_view shader)
	{
		auto handle = g_ShaderHandleAllocator.GetId();
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateShader);

		cmd.WriteObject(handle);
		cmd.WriteString(shader);
		return handle;
	}

	TextureHandle LoadTextureFrom(Memory mem, int w, int h)
	{
		auto handle = g_TextureHandleAllocator.GetId();

		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateTextureFrom);
		cmd.WriteObject(handle);
		cmd.WriteObject(mem);
		cmd.WriteObject(w);
		cmd.WriteObject(h);
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
	
	TextureHandle CreateEmptyTexture(int width, int height, TextureFormat format, ResourceUsage usage,int mipLevels, SampleCount samples, FilterMode min, FilterMode mag, int layers, bool cube)
	{
		auto handle = g_TextureHandleAllocator.GetId();

		 CreateEmptyTextureCmd payload
		 {
			 width,
			 height,
			 format,
			 usage,
			 mipLevels,
			 samples,
			 min,
			 mag,
			 static_cast<uint32_t>(layers),
			 cube,
		};
		 auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateEmptyTexture);
		 cmd.WriteObject(handle);     
		 cmd.WriteObject(payload);
		 return handle;
	}


	void* ReadMappedPtr(ResourceType type,Handle<void>::Type handle)
	{
		return Renderer::Instance()->GetMappedPtr(type,handle);
	}

	void UpdateTexture(TextureHandle handle, uint8_t* data, size_t size)
	{
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::UpdateTexture);
		cmd.WriteObject(handle);
		cmd.WriteObject(data);
		cmd.WriteObject(size);
	}
	size_t ReadTexture(TextureHandle handle, void* data)
	{
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::ReadTexture);
		cmd.WriteObject(handle);
		cmd.WriteObject(data);
		// we are not doing like that:
		// current frame user asks
		// next frame - copy data
		// after that should be available
		return g_Submission->Submit->FrameNumber + 2;
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
		auto item = g_Submission->Submit->CurrentRenderItem;
		auto& bind = item->Bindings.ResBindings[
			item->Bindings.currentResBinding++];

		bind.uniform = name;
		bind.dirty = true;
		bind.handles[bind.currentFreeTextureHandle++] = handle;
		bind.kind = ResourceType::CombinedSampler;
	}
	TextureHandle GetColorTexture(FrameBufferHandle fb, int index)
	{
		auto handle = Renderer::Instance()->GetFBTextureHandle(fb,index);
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

	std::vector<TextureHandle> GenerateMipMapsAsTextures(TextureHandle handle, int mipLevel)
	{
		std::vector<TextureHandle> handles;
		for (auto i = 0; i < mipLevel; i++)
		{
			handles.push_back(g_TextureHandleAllocator.GetId());
		}
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::GenerateMipMapsAsTextures);
		cmd.WriteObject(handle);
		cmd.WriteVector(handles);
		return handles;
	}

	ResourceBindings& GetBindings(Item* item)
	{
		return item->Bindings;
	}

	void BindTextures(std::string_view uniformName, const std::vector<TextureHandle>& handles)
	{
		auto& bindings = GetBindings(g_Submission->Submit->CurrentRenderItem);
		auto& bind = bindings.ResBindings[bindings.currentResBinding++];
		assert(handles.size() <= bind.handles.size());
		bind.uniform = uniformName;
		bind.dirty = true;
		bind.currentFreeTextureHandle += handles.size();
		std::copy_n(handles.begin(),handles.size(), bind.handles.begin());
		bind.kind = ResourceType::CombinedSampler;
	}
	void BindImages(std::string_view uniformName, const std::vector<TextureHandle>& handles)
	{
		auto& bindings = GetBindings(g_Submission->Submit->CurrentRenderItem);
		auto& bind = bindings.ResBindings[bindings.currentResBinding++];
		assert(handles.size() <= bind.handles.size());
		bind.uniform = uniformName;
		bind.dirty = true;
		std::copy_n(handles.begin(), handles.size(), bind.handles.begin());
		bind.currentFreeTextureHandle += handles.size();
		bind.kind = ResourceType::StorageImage;
	}

	void BindImage(std::string_view uniformName, TextureHandle handle)
	{
		auto& bindings = GetBindings(g_Submission->Submit->CurrentRenderItem);

		auto& bind = bindings.ResBindings[bindings.currentResBinding++];

		bind.uniform = uniformName;
		bind.dirty = true;
		bind.handles[bind.currentFreeTextureHandle++] = handle;
		bind.kind = ResourceType::StorageImage;
	}

	void BindBuffer(std::string_view uniformName, BufferHandle handle)
	{
		auto& bindings = GetBindings(g_Submission->Submit->CurrentRenderItem);
		auto& bind = bindings.ResBindings[bindings.currentResBinding++];
		bind.uniform = uniformName;
		bind.dirty = true;
		bind.kind = ResourceType::StorageBuffer;
		bind.buffers[bind.currentFreeBufferHandle++] = handle;

	}
	void BindTexture(std::string_view uniformName, TextureHandle handle)
	{
		
		
		auto& bindings = GetBindings(g_Submission->Submit->CurrentRenderItem);

		auto& bind = bindings.ResBindings[bindings.currentResBinding++];

		bind.uniform = uniformName;
		bind.dirty = true;
		bind.handles[bind.currentFreeTextureHandle++] = handle;
		bind.kind = ResourceType::CombinedSampler;
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

	void SetBlendState(int attachmentIndex, BlendMode state)
	{
		auto item = g_Submission->Submit->CurrentRenderItem;
		if (attachmentIndex <= item->State.blend.size())
		{
			item->State.blend.resize(attachmentIndex + 1);
			item->State.blend[attachmentIndex] = state;
		}
	}

	void SetViewRect(PassID id, size_t x, size_t y, size_t width, size_t height)
	{
		g_Submission->Submit->Views[id].Rect = { x,y,width,height };
	}
	void SubmitCompute(PassID id, ProgramHandle program, size_t x, size_t y, size_t z)
	{
		auto item = g_Submission->Submit->CurrentRenderItem;
		item->Program = program;
		item->GroupCount = { x,y,z };
		item->View = id;

		g_Submission->Submit->Views[id].Type = ItemType::COMPUTE;
		g_Submission->Submit->NextItem(id);
	}
	void Submit(PassID viewID, ProgramHandle programHandle, size_t instances)
	{
		// creates render item
		auto renderItem =g_Submission->Submit->CurrentRenderItem;
		renderItem->ObjectCount = instances;
		renderItem->Program = programHandle;
		renderItem->View =viewID;
		
		g_Submission->Submit->Views[viewID].Type = ItemType::RENDER;
		
		g_Submission->Submit->NextItem(viewID);
		g_Submission->Submit->CurrentRenderItem->MatrixIndex = g_Submission->Submit->CurrentFreeMatrix;
	}

	void SetWindow(SPtr<Window> window)
	{
		g_Submission->Window = window;
	}

	void SubmitInit(InitParams init)
	{
#if THREADING
		g_Submission->Submit = g_Submission->Frames;
		g_Submission->Render = g_Submission->Frames+1;
#else
		g_Submission->Submit = g_Submission->Render = &g_Submission->Frames[0];
#endif
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::RendererInit);
		cmd.WriteObject(init);
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

					Renderer::Instance()->Init(init.width, init.height, g_Submission->Window,	init.app);

					break;
				}
				case Voidstar::ResourceCommand::RendererShutdownBegin:
					break;
				case Voidstar::ResourceCommand::CreateVertexLayout:
					break;
				case ResourceCommand::CreateBuffer:
				{
					auto handle = commandBuffer.ReadObject<BufferHandle>();
					auto mem = commandBuffer.ReadObject<Memory>();
					auto usage = commandBuffer.ReadObject<ResourceUsage>();
					Renderer::Instance()->CreateBuffer(handle, mem, usage);
					break;
				}
				case Voidstar::ResourceCommand::CreateIndexBuffer:
				{
					auto mem = commandBuffer.ReadObject<Memory>();
					auto bufferHandle = commandBuffer.ReadObject<uint16_t>();
					Renderer::Instance()->CreateIndexBuffer(mem, IndexBufferHandle  { bufferHandle });
				}
				break;
				case Voidstar::ResourceCommand::CreateVertexBuffer:
				{
					auto mem = commandBuffer.ReadObject<Memory>();
					auto bufferHandle = commandBuffer.ReadObject<VertexBufferHandle>();
					auto layoutHandle = commandBuffer.ReadObject<uint16_t>();
					auto usage = commandBuffer.ReadObject<ResourceUsage>();

					Renderer::Instance()->CreateVertexBuffer(mem,  bufferHandle, usage);
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
					auto handle = commandBuffer.Read<ShaderHandle>();

					auto path = commandBuffer.ReadString();
					Renderer::Instance()->CompileShader(path);
					break;
				}
				case Voidstar::ResourceCommand::CreateProgram:
				{
					auto handle = commandBuffer.Read<ProgramHandle>();
					auto shaderAmount = commandBuffer.ReadByte();
					Renderer::Instance()->LinkShaders(ProgramHandle{ handle }, shaderAmount);
					break;
				}
				case ResourceCommand::GenerateMipMapsAsTextures:
				{
					auto handle = commandBuffer.Read<TextureHandle>();

					std::vector<TextureHandle> handles;
					commandBuffer.ReadVector<>(handles);
					Renderer::Instance()->CreateEmptyMipMapsAsImages(handle, handles);
					break;
				}

				case ResourceCommand::UpdateBuffer:
				{
					auto handle = commandBuffer.Read<BufferHandle>();
					auto data = commandBuffer.Read<void*>();
					auto size = commandBuffer.Read<size_t>();
					Renderer::Instance()->UpdateBuffer(handle, data, size);
					break;
				}
				case Voidstar::ResourceCommand::CreateTexture:
				{

					auto handle = commandBuffer.ReadObject<TextureHandle>();
					auto path = commandBuffer.ReadString();
					Renderer::Instance()->CreateTexture(handle, path);
					break;
				}
				case Voidstar::ResourceCommand::CreateTextureFrom:
				{

					auto handle = commandBuffer.ReadObject<TextureHandle>();
					auto mem = commandBuffer.ReadObject<Memory>();
					auto w= commandBuffer.ReadObject<int>();
					auto h= commandBuffer.ReadObject<int>();

					Renderer::Instance()->CreateTextureFrom(handle,mem,w,h);
					break;
				}

				case Voidstar::ResourceCommand::CreateEmptyTexture:
				{

					auto handle = commandBuffer.ReadObject<TextureHandle>();
					auto payload = commandBuffer.ReadObject<CreateEmptyTextureCmd>();

					Renderer::Instance()->CreateEmptyTexture(handle, payload);
					break;
				}
				case Voidstar::ResourceCommand::UpdateImageRegionWithImage:
				{
					auto update = commandBuffer.ReadObject<UpdateImageRegion>();
					Renderer::Instance()->UpdateRegionWithImage(update.mem, update.width,	update.height,
						update.imageToUpdate
						, { 0,0,0 },
						update.layer);

					break;
				}
				case Voidstar::ResourceCommand::UpdateTexture:
				{
					auto texture = commandBuffer.ReadObject<TextureHandle>();
					auto pixels = commandBuffer.ReadObject<uint8_t*>();
					auto size = commandBuffer.ReadObject<size_t>();
					Renderer::Instance()->UpdateTexture(texture,pixels,size);
					break;
				}
				case Voidstar::ResourceCommand::FillTexture:
				{
					auto texture = commandBuffer.ReadObject<TextureHandle>();
					auto pixel = commandBuffer.ReadObject<glm::vec4>();
					auto buffer = commandBuffer.ReadObject<BufferHandle>();
					auto offset = commandBuffer.ReadObject<size_t>();
					Renderer::Instance()->FillTexture(texture, pixel, buffer, offset);
					break;
				}
				case Voidstar::ResourceCommand::ResizeTexture:
					break;
				case Voidstar::ResourceCommand::CreateAttachment:
				{
					auto handle = commandBuffer.ReadObject<AttachmentHandle>();
					auto info = commandBuffer.ReadObject<AttachmentInfo_>();
					Renderer::Instance()->CreateAttachment(handle, info);
					break;
				}
				case Voidstar::ResourceCommand::CreateFrameBuffer:
				{
					auto handle = commandBuffer.ReadObject<FrameBufferHandle>();
					auto amount = commandBuffer.ReadObject<size_t>();
					std::vector<AttachmentHandle> handles;
					handles.reserve(amount);
					for (int i = 0; i < amount; i++)
					{
						handles.push_back(commandBuffer.ReadObject<AttachmentHandle>());
					}
					Renderer::Instance()->CreateFramebuffer(handle, handles);
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
				{
					auto handle = commandBuffer.ReadObject<TextureHandle>();
					auto data = commandBuffer.ReadObject<void*>();

					auto image = Renderer::Instance()->GetTexture(handle);
					auto size = image->GetSize();

					auto buffer = Buffer::CreateStagingBuffer(size);


					Renderer::Instance()->CopyImageToBuffer(image, buffer);
					Renderer::Instance()->CopyBufferToPtr(buffer, data, FormatToSize(image->GetFormat()));

					break;
				}
				default:
				{
					assert(false && "unhandled resource command");
					break;
				}

			}
		}
		commandBuffer.Reset();
	}

	//std::mutex m;
	//std::condition_variable cv;
	//bool ready = false;
	std::binary_semaphore renderSem{0};
	std::binary_semaphore apiSem{1};

	void RunRender_(std::atomic_bool& isRunning)
	{

		while (isRunning)
		{
			renderSem.acquire();
			if (!isRunning)
			{
				apiSem.release();
				break;
			}
			// execute prerender commands
			Renderer::Instance()->BeginFrame(g_Submission->Render);
			ExecuteCommands(g_Submission->Render->CmdPre);
			// render commands

			Renderer::Instance()->RenderFrame(g_Submission->Render, g_Submission->Render->deltaTime);

			// execute postrender commands
			ExecuteCommands(g_Submission->Render->CmdPost);
			Renderer::Instance()->EndFrame(g_Submission->Render);
			g_Submission->Render->Reset();
			apiSem.release();
		}
	}

	// start calling implementation
	void ExecuteFrame(float deltaTime, bool wait)
	{
	#if THREADING
		// we wait until renderer is done rendering
		apiSem.acquire();
		 
		
		// swap
		g_Submission->Submit->deltaTime = deltaTime;
		g_Submission->Submit->FrameNumber++;
		std::swap(g_Submission->Submit, g_Submission->Render);

		// signal renderer to do the work
		renderSem.release();
		
		// wait until renderer is finished with previous frame
		if (wait)
		{
			apiSem.acquire();
			apiSem.release();
		}
		// unless specified multithreaded, render one this thread
	#else

		std::swap(g_Submission->Submit, g_Submission->Render);
		g_Submission->Submit->FrameNumber++;
		// execute prerender commands
		Renderer::Instance()->BeginFrame(g_Submission->Render);
		ExecuteCommands(g_Submission->Render->CmdPre);
		// render commands

		Renderer::Instance()->RenderFrame(g_Submission->Render, deltaTime);

		// execute postrender commands
		ExecuteCommands(g_Submission->Render->CmdPost);
		Renderer::Instance()->EndFrame(g_Submission->Render);
		g_Submission->Render->Reset();

	#endif
		
	}
	void BindIndexBuffer(IndexBufferHandle handle)
	{
		auto item = g_Submission->Submit->CurrentRenderItem;
		item->IndexBuffer = handle;
	};
	
	void SetRenderMode(RenderMode mode)
	{
		auto item = g_Submission->Submit->CurrentRenderItem;
		item->renderMode = mode;
	}
	void SetClipRect(int x, int y, int w, int h)
	{
		auto item = g_Submission->Submit->CurrentRenderItem;
		item->ClipRect = glm::vec4{x,y,w,h};
	}

	void SetDepthWrite(bool write)
	{
		g_Submission->Submit->CurrentRenderItem->State.depthWrite = write;
	}
	void SetDepthTest(bool test)
	{
		g_Submission->Submit->CurrentRenderItem->State.depthTest = test;
	}
	void SetTransform(const glm::mat4& world)
	{
		
		g_Submission->Submit->Matricies[g_Submission->Submit->CurrentFreeMatrix++] = world;
	};
	void BindVertexBuffer(uint16_t location, VertexBufferHandle handle, VertexStreamMode mode)
	{
		auto item = g_Submission->Submit->CurrentRenderItem;
		item->VertexBindings[location].Mode = mode;
		item->VertexBindings[location].VertexHandle = handle;
		item->VertexBindings[location].LayoutHandle = g_Submission->VertexLayoutMap.at(handle);
		item->Bindings.vertexCurrentBinding++;
	};


	VertexBufferHandle CreateVertexBuffer(Memory mem, VertexLayout& layout, ResourceUsage usage)
	{
		auto bufferHandle = VertexBufferHandle{ g_VertexBufferHandleAllocator.GetId() };
		

		auto layoutHandle = VertexLayoutHandle{g_LayoutHandleAllocator.GetId()};
		g_Submission->Layouts.insert({layoutHandle,layout});
		g_Submission->VertexLayoutMap.insert({ bufferHandle, layoutHandle });

		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateVertexBuffer);
		cmd.WriteObject(mem);
		cmd.WriteObject(bufferHandle);
		cmd.WriteObject(layoutHandle);
		cmd.WriteObject(usage);

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

	BufferHandle CreateBuffer(Memory mem, ResourceUsage usage)
	{
		auto bufferHandle = g_BufferHandleAllocator.GetId();

		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::CreateBuffer);
		cmd.WriteObject(bufferHandle);
		cmd.WriteObject(mem);
		cmd.WriteObject(usage);

		return bufferHandle;

	}


	void UpdateImageRegionWithImage(
		const Memory& loadedImage,
		size_t width,
		size_t height,
		TextureHandle imageToUpdate,
		glm::vec3 offset,
		int layer)
	{
		UpdateImageRegion update{ loadedImage,width,height,imageToUpdate,offset,layer };
		
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::UpdateImageRegionWithImage);
		cmd.WriteObject(update);
		
	}
	void SetData(BufferHandle buffer, void* data,size_t size)
	{
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::UpdateBuffer);
		cmd.WriteObject(buffer);
		cmd.WriteObject(data);
		cmd.WriteObject(size);
	}
	void FillImage(TextureHandle handle, const glm::vec4& pixel,BufferHandle buffer, size_t bufferOffset)
	{
		auto& cmd = g_Submission->GetCommandBuffer(ResourceCommand::FillTexture);
		cmd.WriteObject(handle);
		cmd.WriteObject(pixel);
		cmd.WriteObject(buffer);
		cmd.WriteObject(bufferOffset);
	}

	size_t GetSize(TextureHandle handle)
	{
		return Renderer::Instance()->GetSize(handle);
	}


	static const cgltf_accessor* FindAttr(const cgltf_primitive* prim, const char* name)
	{
		for (cgltf_size i = 0; i < prim->attributes_count; i++)
		{
			const cgltf_attribute& a = prim->attributes[i];
			if (a.name && 0 == std::strcmp(a.name, name))
				return a.data; 
		}
		return nullptr;
	}

	static TextureHandle LoadGLTFTexture(cgltf_image* img, const std::filesystem::path& basePath)
	{
		if (!img) { assert(false); }

		stbi_uc* pixels = nullptr;
		int w = 0, h = 0, comp = 0;

		if (img->buffer_view)
		{
			// Embedded in .glb / buffer
			const cgltf_buffer_view* bv = img->buffer_view;
			const cgltf_buffer* buf = bv->buffer;
			assert(buf && buf->data && "Call cgltf_load_buffers first!");
			const uint8_t* bytes = (const uint8_t*)buf->data + bv->offset;
			const int byteCount = (int)bv->size;
			pixels = stbi_load_from_memory(bytes, byteCount, &w, &h, &comp, 4);
		}
		else if (img->uri)
		{
			// External file referenced by relative path
			std::filesystem::path imgPath = basePath.parent_path() / img->uri;
			pixels = stbi_load(imgPath.string().c_str(), &w, &h, &comp, 4);
			assert(pixels && "Failed to load external texture");
		}

		if (!pixels) { assert(false); }

		Memory mem;
		mem.size = w * h * 4;
		mem.allocate = AllocateWay::STBI;
		mem.cleanUp = true;
		mem.data = pixels;
		auto tex = LoadTextureFrom(mem, w, h);
		return tex;
	}


//https://gamedev.stackexchange.com/questions/68612/how-to-compute-tangent-and-bitangent-vectors
	static void CalculateTangents(std::vector<VertexModel_>& vertices,
		const std::vector<IndexType>& indexes)
	{
		size_t vertexCount = vertices.size();
		std::vector<glm::vec3> tan1(vertexCount, glm::vec3(0.0f));
		std::vector<glm::vec3> tan2(vertexCount, glm::vec3(0.0f));

		for (size_t i = 0; i < indexes.size(); i += 3)
		{
			uint32_t i1 = indexes[i + 0];
			uint32_t i2 = indexes[i + 1];
			uint32_t i3 = indexes[i + 2];

			const glm::vec3& v1 = vertices[i1].Position;
			const glm::vec3& v2 = vertices[i2].Position;
			const glm::vec3& v3 = vertices[i3].Position;

			const glm::vec2& w1 = vertices[i1].UV;
			const glm::vec2& w2 = vertices[i2].UV;
			const glm::vec2& w3 = vertices[i3].UV;

			float x1 = v2.x - v1.x;
			float x2 = v3.x - v1.x;
			float y1 = v2.y - v1.y;
			float y2 = v3.y - v1.y;
			float z1 = v2.z - v1.z;
			float z2 = v3.z - v1.z;

			float s1 = w2.x - w1.x;
			float s2 = w3.x - w1.x;
			float t1 = w2.y - w1.y;
			float t2 = w3.y - w1.y;

			float denom = (s1 * t2 - s2 * t1);
			float r = (std::abs(denom) < 1e-8f) ? 0.0f : 1.0f / denom;

			glm::vec3 sdir(
				(t2 * x1 - t1 * x2) * r,
				(t2 * y1 - t1 * y2) * r,
				(t2 * z1 - t1 * z2) * r);
			glm::vec3 tdir(
				(s1 * x2 - s2 * x1) * r,
				(s1 * y2 - s2 * y1) * r,
				(s1 * z2 - s2 * z1) * r);

			tan1[i1] += sdir;
			tan1[i2] += sdir;
			tan1[i3] += sdir;

			tan2[i1] += tdir;
			tan2[i2] += tdir;
			tan2[i3] += tdir;
		}

		for (size_t a = 0; a < vertexCount; a++)
		{
			const glm::vec3& n = vertices[a].Normal;
			const glm::vec3& t = tan1[a];

			// Gram-Schmidt orthogonalize
			glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));
			float len = glm::length(tangent);
			// Handedness
			float w = (glm::dot(glm::cross(n, t), tan2[a]) < 0.0f) ? -1.0f : 1.0f;

			vertices[a].Tangent = glm::vec4(tangent, w);
			//vertices[a].Tangent = glm::vec4(2, 2, 2, 7);
		}
	}

	SPtr<Model> LoadModel(std::string_view file)
	{
		using namespace std::filesystem;
		auto resPath = BASE_RES_PATH;
		path pathFile{ resPath.append(file)};
		assert(std::filesystem::exists(pathFile));

		SPtr<Model> model = CreateSPtr<Model>();

		cgltf_options options{};
		cgltf_data* data = NULL;
		// reading description of the model 
		cgltf_result result = cgltf_parse_file(&options, pathFile.string().c_str(), &data);
		if (result == cgltf_result_success)
		{
			//Note that contents of external files for buffers and images are not
			//automatically loaded

			result = cgltf_load_buffers(&options, data, pathFile.string().c_str());


			cgltf_mesh* mesh = &data->meshes[0];
			cgltf_primitive* prim = &mesh->primitives[0];

			auto pos = FindAttr(prim, "POSITION");
			auto nrm = FindAttr(prim, "NORMAL");
			auto tgt = FindAttr(prim, "TANGENT");
			auto uv = FindAttr(prim, "TEXCOORD_0");


			cgltf_material* m = prim->material; 
			if (m)
			{
				// Albedo / base color
				if (m->pbr_metallic_roughness.base_color_texture.texture)
					model->Albedo = LoadGLTFTexture(
						m->pbr_metallic_roughness.base_color_texture.texture->image, pathFile);

				// Normal map
				if (m->normal_texture.texture)
					model->Normal = LoadGLTFTexture(m->normal_texture.texture->image, pathFile);
				
				// Metallic/Roughness (packed: G=roughness, B=metallic in glTF spec)
				if (m->pbr_metallic_roughness.metallic_roughness_texture.texture)
					model->Metallic = LoadGLTFTexture(
						m->pbr_metallic_roughness.metallic_roughness_texture.texture->image, pathFile);

				// Ambient occlusion
				//m->occlusion_texture.texture->image

				// Emissive
				//m->emissive_texture.texture->image
			}

			size_t vCount = pos->count;
			std::vector<VertexModel_> vertices;
			vertices.resize(vCount);
			for (cgltf_size i = 0; i < pos->count; i++)
			{
				cgltf_accessor_read_float(pos, i, &vertices[i].Position.x, 3);

				if (nrm) cgltf_accessor_read_float(nrm, i, &vertices[i].Normal.x, 3);
				else 
				{
					vertices[i].Normal[0] = 0;
					vertices[i].Normal[1] = 1;
					vertices[i].Normal[2] = 0;
				}

				if (uv)
				{
					cgltf_accessor_read_float(uv, i, &vertices[i].UV.x, 2);
				}
				else 
				{ 
					vertices[i].UV[0] = 0;
					vertices[i].UV[1] = 0;
				}

			}

			std::vector<IndexType> indexes;

			if (prim->indices) {
				indexes.resize(prim->indices->count);
				for (size_t i = 0; i < indexes.size(); i += 1)
				{
					indexes[i] = (IndexType)cgltf_accessor_read_index(prim->indices, i);
				}
				if (tgt)
				{
					for (cgltf_size i = 0; i < pos->count; i++)
						cgltf_accessor_read_float(tgt, i, &vertices[i].Tangent.x, 4);
				}
				else
				{
					CalculateTangents(vertices, indexes);
				}
			}

			VertexLayout layout;
			layout.AddVertex(ShaderDataType::FLOAT3);
			layout.AddVertex(ShaderDataType::FLOAT3);
			layout.AddVertex(ShaderDataType::FLOAT2);
			layout.AddVertex(ShaderDataType::FLOAT4);

			Memory mem;
			mem.data = reinterpret_cast<uint8_t*>(vertices.data());
			mem.size = vertices.size() * sizeof(vertices[0]);
			model->m_VertexBuffer = CreateVertexBuffer(mem,layout);
			
			mem.data = reinterpret_cast<uint8_t*>(indexes.data());
			mem.size = indexes.size() * sizeof(indexes[0]);
			model->m_IndexBuffer = CreateIndexBuffer(mem);

			ExecuteFrame(0,true);


			cgltf_free(data);
		}

		return model;
	}
	void SubmitModel(SPtr<Model> model, PassID pass, ProgramHandle program, const glm::mat4& world)
	{
		BindVertexBuffer(pass, model->m_VertexBuffer);
		BindIndexBuffer(model->m_IndexBuffer);
		BindTexture("u_Albedo", model->Albedo);
		BindTexture("u_Normal", model->Normal);
		BindTexture("u_Metallic", model->Metallic);
		g_Submission->Submit->CurrentRenderItem->MatrixIndex = g_Submission->Submit->CurrentFreeMatrix++;
		g_Submission->Submit->Matricies[g_Submission->Submit->CurrentRenderItem->MatrixIndex] = world;
		Submit(pass, program);
		return;
	}

	void Frame::NextItem(PassID viewID)
	{	
		auto& freeIndex = g_Submission->Submit->Views[viewID].FreeIndex;
		g_Submission->Submit->Views[viewID].ItemsIndex[freeIndex++] = g_Submission->Submit->CurrentRenderItemIndex;

		if (g_Submission->Submit->LastView.size() == 0)
		{
			g_Submission->Submit->LastView.push_back(viewID);
		}
		// so can have chains of items per view in frame
		else if (g_Submission->Submit->LastView.back() != viewID)
		{
			g_Submission->Submit->LastView.push_back(viewID);
		}

		CurrentRenderItemIndex++;
		CurrentRenderItem = &m_renderItem[CurrentRenderItemIndex];
	}
}



