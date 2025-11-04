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
#include "RenderPassGraph.h"
#include "Drawables.h"
#include"tracy/Tracy.hpp"
#include"tracy/TracyVulkan.hpp"


#include "CommandPoolManager.h"
#include "IndexBuffer.h"

#include "ShaderType.h"
#include "Vertex_.h"
#include "Handle.h"
#include "SparseSet.h"
#include "ShaderCompiler.h"
#include "AttachmentManager.h"

#include <array>





struct ImGui_ImplVulkanH_Window;
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
	inline std::string BASE_RES_PATH = "res";
	inline std::string BASE_VIRT_PATH = "E:/dev/Voidstar/mipMaps_virtualTex4.tiff/";
	const std::string SPIRV_COMPILER_PATH = std::string(std::string(std::getenv("VULKAN_SDK")) + std::string("/Bin/glslangvalidator.exe"));

	inline std::string BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";
	inline std::string InitFilePath()
	{
		std::string baseShaderPath = "";

		// Check if running within Visual Studio
		const char* visualStudioEnvVar = std::getenv("VSLANG");
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
			BASE_SHADER_PATH = executablePath.parent_path().string() + "../../../Shaders/";
			BASE_RES_PATH = executablePath.parent_path().string() + "../../../res/";
			BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";
			BASE_VIRT_PATH = executablePath.parent_path().string() + "../../../../mipMaps_virtualTex4.tiff/";
		}
		return baseShaderPath;
	}




	struct VOIDSTAR_API UniformBufferObject {
		glm::mat4 view;
		glm::mat4 proj;
		alignas(4)float time;
	};



	class Instance;
	class Device;
	class Swapchain;
	class Application;
	class DescriptorSetLayout;
	class CommandPoolManager;
	class VOIDSTAR_API Renderer
	{
		typedef std::unordered_map<std::pair<int, PipelineType>, std::vector < vk::DescriptorSetLayoutBinding>, EnumClassHash>
			Bindings;
		typedef std::unordered_map<std::pair<int, PipelineType>, int, EnumClassHash>
			Sets;

	public:
		void Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app);
		static Renderer* Instance();
		
		void BeginFrame(Camera& camera, size_t viewportWidth, 
			size_t viewportHeight);
		void RenderFrame(Frame* render, float deltaTime);
		void CompileShader(std::string_view shader);
		void LinkShaders(ProgramHandle handle, uint8_t shaderAmount);

		void CreateTexture(TextureHandle handle, std::string_view path);
		void CreateUniform(UniformHandle handle, ResourceType type, size_t num) {};
		void CreateAttachment(AttachmentHandle handle, AttachmentInfo_ info);
		void CreateFramebuffer(FrameBufferHandle handle, const std::vector<AttachmentHandle>& info);
		void CreateVertexBuffer(Memory& mem, VertexBufferHandle vertHandle, UpdateHint hint = UpdateHint::Static);
		void CreateIndexBuffer(Memory& mem, IndexBufferHandle indexHandle);
		void CreatePipelineLayout(PipelineLayoutKey& key);
		vk::DescriptorSetLayout CreateDescriptorLayout(const DescriptorLayoutKey& key);
		
		TextureHandle GetFBTextureHandle(FrameBufferHandle fb);
		SPtr<Image> GetTexture(TextureHandle handle);



		void EndFrame();
		void UserInit();

		CommandPoolManager* GetCommandPoolManager()
		{
			return m_CommandPoolManager.get();
		}
		void Flush(std::vector< vk:: CommandBuffer > commandBuffers);
		void Wait(const vk::Fence& fence);
		void Reset(const vk::Fence& fence);
		~Renderer();
		Sets& GetSets() { return m_SetsAmount; }
		Bindings& GetBindings() { return m_Bindings; }
		template<typename T>
		const T GetSet(int handle, PipelineType type)
		{
			return std::get<T>(m_Sets[{handle, type}]);;
		}
		const DescriptorSetLayout* GetSetLayout(int handle, PipelineType type)
		{
			return m_Layout[{handle, type}];
		}
		std::pair<float, float> GetViewportSize() const { return { m_ViewportWidth,m_ViewportHeight }; }
		vk::PolygonMode GetPolygonMode() const { return m_PolygoneMode; }
		void Shutdown();
		CommandBuffer& GetRenderCommandBuffer(size_t frameindex);
		CommandBuffer& GetComputeCommandBuffer(size_t frameindex);
		CommandBuffer& GetTransferCommandBuffer(size_t frameindex);
		
		// position 0 0 is center of screen
		void DrawQuadScreen(vk::CommandBuffer commandBuffer);
		void DrawQuad(glm::mat4& world, glm::vec4 color);
		void DrawQuad(std::vector<Vertex_>& verticies);
		void DrawTxt(vk::CommandBuffer commandBuffer, std::string_view str,glm::vec2 pos, std::map<unsigned char, Character> &Characters);
		void Draw(Drawable& drawable);
		void DrawSphere(glm::vec3 pos, glm::vec3 scale,
			glm::vec4 color, glm::vec3 rot);
		void DrawSphereInstance(vk::CommandBuffer& commandBuffer);
		void BeginBatch();
		void DrawBatch(vk::CommandBuffer& commandBuffer,size_t offset = 0, int index = 0);
		void DrawBatchCustom(vk::CommandBuffer& commandBuffer, size_t indexAmount, size_t offset = 0, int index = 0);
		vk::Fence GetFence()
		{
			return m_InFlightFence[m_CurrentFrame].GetFence();
		}
		UPtr<Buffer> m_QuadBufferBatch{ nullptr };
		UPtr<IndexBuffer> m_QuadBufferBatchIndex{ nullptr };
		UPtr<Buffer> m_InstanceBuffer{ nullptr };
		
		Vertex_ * m_BatchQuad;
		Vertex_ * m_BatchQuadStart;
		//InstanceData* m_BatchInstance; 
		//InstanceData* m_BatchInstanceStart;
		
		int m_QuadIndex= 0;
		void CreateSyncObjects();
		void AddRenderGraph(std::string_view name, UPtr<RenderPassGraph> graph)
		{
			m_Graphs.emplace_back(std::move(graph));
		}
		void AddDrawable(std::string_view renderPassName, const Drawable& drawable)
		{
			auto& drawables = m_Drawables[renderPassName.data()];
			drawables.push_back(drawable);
		}
		void AddStaticDrawable(std::string_view renderPassName, const Drawable& drawable)
		{
			auto& drawables = m_StaticDrawables[renderPassName.data()];
			drawables.push_back(drawable);
		}

		std::vector<Drawable>& GetDrawables(std::string_view renderPassName)
		{
			auto& drawables = m_Drawables.at(renderPassName.data());
			return drawables;
		}
		std::vector<Drawable>& GetStaticDrawables(std::string_view renderPassName)
		{
			auto& drawables = m_StaticDrawables.at(renderPassName.data());
			return drawables;
		}
		std::vector<UPtr<Buffer>> m_UniformBuffers;
		void Draw(Quad& quad, glm::mat4& world);
		void Draw(Sphere& drawable);
		void Draw(QuadRangle& drawable);
		void UpdateUniformBuffer(const glm::mat4& proj, const glm::mat4& view,float time);
		void AddFramebuffers(FrameBufferHandle handle, std::vector<vk::Framebuffer>& framebuffers);
		size_t m_CurrentFrame = 0;
	private:
		vk::Pipeline GetPipeline(const PipelineKey& key, std::array<VertexBinding, RenderItem::MAX_VERTEX_BINDING>& bindings,
			int bindingAmount);
		void CreateInstance();
		void RecreateSwapchain();
		std::vector<vk::DescriptorSet>  AllocateSets(size_t amount, const DescriptorLayoutKey& key);
		void CreateLayouts();
		void CleanUpLayouts();
		SparseSet<RenderPassHandle_> g_RenderPassAllocator;
	private:
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
		std::vector<void*> m_UniformBuffersMapped;

		DescriptorLayoutKey SystemDescriptorLayoutKey;

		Map<DescriptorLayoutKey, vk::DescriptorSetLayout, DescriptorLayoutKeyHash> m_DescriptorLayout;

		Map<PipelineLayoutKey, vk::PipelineLayout, PipelineLayoutKeyHash> m_PipelineLayout;
		Map<PipelineKey, vk::Pipeline, PipelineKeyHash> m_Pipelines;

		Map<DescriptorLayoutKey, std::vector<vk::DescriptorSet>, DescriptorLayoutKeyHash> m_DescriptorSet;

		Map<VertexBufferHandle, SPtr<Buffer>> m_VertexBuffers;
		Map<IndexBufferHandle, SPtr<IndexBuffer>> m_IndexBuffers;
		//unordered_map<VertexLayoutHandle, > m_BufferLayouts;

		Map<FrameBufferHandle, std::vector<vk::Framebuffer>> m_Framebuffers;
		Map<FrameBufferHandle, std::vector<AttachmentHandle>> m_FBAttachments;

		Map<AttachmentHandle, AttachmentInfo_> m_AttachmentInfo;

		// 0 handle is default render pass
		Map<FrameBufferHandle, RenderPass> m_RenderPasses;



		Map<TextureHandle, SPtr<Image>> m_Textures;

		struct UpdateBack
		{
			TextureHandle texture;
			vk::ImageLayout to;
		};
		std::vector<std::stack<UpdateBack>> updateBack;


		std::vector<CommandBuffer> m_RenderCommandBuffer,
			m_TransferCommandBuffer, m_ComputeCommandBuffer;
		vk::CommandPool m_FrameCommandPool;
		
				




		SPtr<DescriptorPool> m_UniversalPool;


		UPtr<CommandPoolManager> m_CommandPoolManager;
		

		std::vector<Semaphore> m_ImageAvailableSemaphore;
		std::vector<Semaphore> m_RenderFinishedSemaphore;
		std::vector<Fence> m_InFlightFence;

		std::vector<Semaphore> m_ComputeFinishedSemaphores;
		std::vector<Fence> m_ComputeInFlightFences;

		SPtr<Window> m_Window;

		vk::PolygonMode m_PolygoneMode = vk::PolygonMode::eFill;


		// int is number of set, int is a type of pipeline render or compute
		Bindings m_Bindings;
		std::unordered_map<std::pair<int, PipelineType>, DescriptorSetLayout*, EnumClassHash> m_Layout;
		std::unordered_map<std::pair<int, PipelineType>, std::variant<vk::DescriptorSet, std::vector<vk::DescriptorSet> >, EnumClassHash> m_Sets;

		Sets m_SetsAmount;
		
		std::vector<UPtr<RenderPassGraph>> m_Graphs;
		std::unordered_map<std::string,std::vector<Drawable>> m_Drawables;
		std::unordered_map<std::string,std::vector<Drawable>> m_StaticDrawables;

		
	
	};

}

