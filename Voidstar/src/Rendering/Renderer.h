#pragma once
#include "../Prereq.h"
#include "vulkan/vulkan.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include"CommandBuffer.h"
#include"../Types.h"
#include"QuadTree/QuadTree.h"
#include"BinderHelp.h"
#include<variant>
#include<map>
#include"Font.h"
#include <functional>
#include "Sync.h"
#include "Pipeline.h"

#include "Drawables.h"
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"

#include "RenderPass.h"
#include "CommandPoolManager.h"
#include "IndexBuffer.h"

#include "ShaderType.h"
#include "Vertex_.h"
#include "Handle.h"
#include "SparseSet.h"
#include "ShaderCompiler.h"
#include "AttachmentManager.h"

#include <array>
#include <print>





namespace Voidstar
{
	struct DynamicKey
	{
		ResourceType kind;
		Handle<void>::Type idx;

		bool operator==(const DynamicKey& other) const {
			return kind == other.kind && idx == other.idx;
		}

		bool operator!=(const DynamicKey& other) const {
			return !(*this == other);
		}

	};
}


namespace std {
	template<>
	struct hash<Voidstar::DynamicKey>
	{
		size_t operator()(const Voidstar::DynamicKey& k) const noexcept
		{
			const auto kind_u = static_cast<std::size_t>(k.kind);

			const auto idx_u = static_cast<std::size_t>(k.idx);

			std::size_t h1 = std::hash<std::size_t>{}(kind_u);
			std::size_t h2 = std::hash<std::size_t>{}(idx_u);
			return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
		}
	};
} // name

namespace Voidstar
{
	class Window;
	class SwapChainSupportDetails;
	class Buffer;
	class Camera;
	struct Frame;
	class Image;
	class DescriptorPool;
	class Model;
	class Pipeline;



	inline std::string BASE_SHADER_PATH = "../Shaders/";
	inline std::string BASE_RES_PATH = "../res/";
	
	const std::string SPIRV_COMPILER_PATH = std::string(std::string(std::getenv("VULKAN_SDK")) + std::string("/Bin/glslangvalidator.exe"));

	inline std::string BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";
	inline std::string BASE_VIRT_PATH = "E:/dev/Voidstar/mipMaps_virtualTex4.tiff/";
	inline std::string InitFilePath()
	{
		std::string baseShaderPath = "";

		// Check if running within Visual Studio
		const char* visualStudioEnvVar = std::getenv("VSLANG");
		//std::cout << (visualStudioEnvVar != nullptr) ? 1 : 0;
		if (visualStudioEnvVar != nullptr)
		{
			// Set the base shader path relative to the project directory
			BASE_SHADER_PATH = "../Shaders/";
			BASE_RES_PATH = "../res/";

		}
		else
		{
			// Set the base shader path relative to the executable directory
			std::filesystem::path executablePath = std::filesystem::current_path();
			BASE_SHADER_PATH = executablePath.parent_path().string() + "/../../Shaders/";
			BASE_RES_PATH = executablePath.parent_path().string() + "/../../res/";
			BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";
			BASE_VIRT_PATH = "E:/dev/Voidstar/mipMaps_virtualTex4.tiff/";
			//BASE_VIRT_PATH = executablePath.parent_path().string() + "/../../mipMaps_virtualTex4.tiff/";
		}
		return baseShaderPath;
	}




	struct VOIDSTAR_API UniformBufferObject {
		glm::mat4 view;
		glm::mat4 proj;
		float time;
		alignas(16)glm::mat4 uiProj;
	};


	struct Font
	{
		SPtr<Image> Atlas;
		int LineSpacing;
		std::unordered_map<unsigned char, Character> Characters;
	};

	class Instance;
	class Device;
	class Swapchain;
	class Application;
	class DescriptorSetLayout;
	class CommandPoolManager;

	struct Stats
	{
		int descriptorsSetsAllocated = 0;
	};
	class VOIDSTAR_API Renderer
	{
	public:
		void Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app);
		static Renderer* Instance();
		
		void UpdateTexture(TextureHandle handle, uint8_t* data, size_t size);
		void RenderFrame(Frame* render, float deltaTime);
		void CompileShader(std::string_view shader);
		void LinkShaders(ProgramHandle handle, uint8_t shaderAmount);
		void CreateEmptyTexture(TextureHandle handle, const CreateEmptyTextureCmd& cmd);

		// sus 
		void CreateEmptyMipMapsAsImages(TextureHandle handle, std::vector <TextureHandle>& handles);
		//
		
		void CreateTexture(TextureHandle handle, std::string_view path);
		void CreateTextureFrom(TextureHandle handle, Memory& mem, int w, int h);
		void CreateAttachment(AttachmentHandle handle, AttachmentInfo_ info);
		void CreateFramebuffer(FrameBufferHandle handle, const std::vector<AttachmentHandle>& info);
		void LoadFont(FontHandle handle, std::string path);

		void CreateBuffer(BufferHandle handle, Memory mem, ResourceUsage usage);
		
		void HandleDynamic(Voidstar::ResourceUsage usage, ResourceType type, Handle<void>::Type handle, int& frames);
		void HandleMapped(Handle<void>::Type idx, ResourceType type, vk::DeviceMemory mem,
			size_t size, ResourceUsage usage);
		
		void CreateVertexBuffer(Memory& mem, VertexBufferHandle vertHandle,
			ResourceUsage hint = ResourceUsage::Vertex);
		void CreateIndexBuffer(Memory& mem, IndexBufferHandle indexHandle);
		void CreatePipelineLayout(PipelineLayoutKey& key);
		vk::DescriptorSetLayout CreateDescriptorLayout(const DescriptorLayoutKey& key);
		void CopyImageToBuffer(SPtr<Image> image, SPtr<Buffer> buffer);
		TextureHandle GetFBTextureHandle(FrameBufferHandle fb, int index);
		void UpdateBuffer(BufferHandle handle, void* data,size_t size);
		SPtr<Image> GetTexture(TextureHandle handle);

		void FillTexture(TextureHandle texture, glm::vec4& pixel,BufferHandle buffer, size_t offset);
		void CopyBufferToPtr(SPtr<Buffer> buffer, void* data, size_t offset);

		void* GetMappedPtr(ResourceType type, Handle<void>::Type handle);
		
		void BeginFrame(Frame* frame);
		void EndFrame(Frame* frame);
		

		CommandPoolManager* GetCommandPoolManager()
		{
			return m_CommandPoolManager.get();
		}
		void Wait(const vk::Fence& fence);
		void Reset(const vk::Fence& fence);
		~Renderer();
	
	
		std::pair<float, float> GetViewportSize() const { return { m_ViewportWidth,m_ViewportHeight }; }
		
		void Shutdown();
		CommandBuffer& GetRenderCommandBuffer(size_t frameindex);
		CommandBuffer& GetComputeCommandBuffer(size_t frameindex);
		CommandBuffer& GetTransferCommandBuffer(size_t frameindex);
		

		void DrawTxt(vk::CommandBuffer commandBuffer, std::string_view str,glm::vec2 pos, std::map<unsigned char, Character> &Characters);
		
		vk::Fence GetFence()
		{
			return m_InFlightFence[m_CurrentFrame].GetFence();
		}
		UPtr<Buffer> m_QuadBufferBatch{ nullptr };
		UPtr<IndexBuffer> m_QuadBufferBatchIndex{ nullptr };
		UPtr<Buffer> m_InstanceBuffer{ nullptr };
		
		int m_QuadIndex = 0;
		void CreateSyncObjects();
		
		std::vector<UPtr<Buffer>> m_UniformBuffers;
		std::vector<void*> m_UniformBuffersMapped;

		std::vector<UPtr<Buffer>> m_ObjectsBuffers;
		std::vector<void*> m_ObjectsBuffersMapped;

		std::vector<UPtr<Buffer>> m_BatchQuadBuffers;
		// start and current
		std::vector<std::pair<Vertex_*, Vertex_*>> m_BatchQuadBuffersMapped;

		Stats stats;

		void UpdateUniformBuffer(const glm::mat4& proj, const glm::mat4& view,float time, const glm::mat4& uiProj);
		void AddFramebuffers(FrameBufferHandle handle, std::vector<vk::Framebuffer>& framebuffers);
		size_t m_CurrentFrame = 0;


		void UpdateRegionWithImage(Memory& mem, size_t width, size_t height, TextureHandle image, vk::Offset3D offset, int layer);
		size_t GetSize(TextureHandle handle);
		void RecreateSwapchain();

		DescriptorLayoutKey SystemDescriptorLayoutKey;
	private:
		SPtr<Buffer> GetBuffer(BufferHandle handle, int currentFrame);
		void PrepareDescritptors( View& view, Frame* render, CommandBuffer cmd, uint32_t& currentMatrixOffset);
		vk::DescriptorSet GetDescriptorSet(DescriptorLayoutKey& key, int frameIndex, int itemIndex);
		void UpdateDescriptors(vk::PipelineBindPoint bindPoint, Item& item, std::vector<DescriptorLayoutKey>& keys, ProgramMeta& meta, CommandBuffer& cmd, vk::PipelineLayout layout,int itemIndex);
		vk::Pipeline GetComputePipeline(PipelineKey& key);
		vk::Pipeline GetPipeline(const PipelineKey& key, std::array<VertexBinding, Item::MAX_VERTEX_BINDING>& bindings,
			int bindingAmount);
		void CreateInstance();
		int GetIndex(FrameBufferHandle handle, bool lastRenderItem);
		SparseSet<RenderPassHandle_> g_RenderPassAllocator;
	private:
		
		uint32_t m_SwapchainIndex = 0;
		bool     m_IsSwapchainAcquired = false;
		Application* m_App;
		Voidstar::Instance* m_Instance;
		Device* m_Device;
		int m_ViewportWidth, m_ViewportHeight;
		AttachmentManager m_AttachmentManager;
		friend class AttachmentManager;
		AttachmentHandle m_DefaultColorAttachment;
		AttachmentHandle m_DefaultMSAAAttachment;
		AttachmentHandle m_DefaultDepthAttachment;
		
		ShaderCompiler m_Compiler;

		FrameBufferHandle DEFAULT_FRAME_BUFFER;
	


		Map<DescriptorLayoutKey, vk::DescriptorSetLayout, DescriptorLayoutKeyHash> m_DescriptorLayout;

		Map<PipelineLayoutKey, vk::PipelineLayout, PipelineLayoutKeyHash> m_PipelineLayout;
		Map<PipelineKey, vk::Pipeline, PipelineKeyHash> m_Pipelines;


		// what frame uses which descirptor layouts and what render items in the frame
		// use what descirptor set
		// each new set is a different DescriptorLayoutKey
		Map<DescriptorLayoutKey, 
			std::vector<Map<int, vk::DescriptorSet>>,
			DescriptorLayoutKeyHash> m_DescriptorSet;

		std::vector<TextureHandle> m_ColorSwapchainHandles;
		
		Map<VertexBufferHandle, std::vector<SPtr<Buffer>>> m_VertexBuffers;
		Map<DynamicKey, bool> m_Dynamic;
		// cache mapped ptrs
		Map<DynamicKey, std::vector<void*>> m_Mapped;



		Map<IndexBufferHandle, SPtr<IndexBuffer>> m_IndexBuffers;
		//unordered_map<VertexLayoutHandle, > m_BufferLayouts;

		Map<FrameBufferHandle, std::vector<vk::Framebuffer>> m_Framebuffers;
		Map<FrameBufferHandle, std::vector<AttachmentHandle>> m_FBAttachments;

		Map<AttachmentHandle, AttachmentInfo_> m_AttachmentInfo;

		// 0 handle is default render pass
		Map<FrameBufferHandle, RenderPass> m_RenderPasses;

		Map<BufferHandle, std::vector<SPtr<Buffer>>> m_Buffers;

		Map<TextureHandle, SPtr<Image>> m_Textures;
		
		Map<FontHandle, Font> m_Fonts;
		
		
		std::vector<CommandBuffer> m_RenderCommandBuffer,
			m_TransferCommandBuffer, m_ComputeCommandBuffer;
		vk::CommandPool m_FrameCommandPool;
		
		SPtr<DescriptorPool> m_UniversalPool;

		UPtr<CommandPoolManager> m_CommandPoolManager;
		
		std::vector<Semaphore> m_ImageAvailableSemaphore;
		std::vector<Semaphore> m_RenderFinishedSemaphore;
		std::vector<Semaphore> m_TimelineSemaphore;


		std::vector<Fence> m_InFlightFence;


		SPtr<Window> m_Window;
	};

}

