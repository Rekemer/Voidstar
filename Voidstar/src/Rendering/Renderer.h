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
#include "ShaderCompiler.h"
#include "AttachmentManager.h"

struct ImGui_ImplVulkanH_Window;
namespace Voidstar
{
	class Window;
	class SwapChainSupportDetails;
	class Buffer;
	class Camera;
	class Application;
	class Image;
	class DescriptorPool;
	class Model;
	class Pipeline;
	struct VOIDSTAR_API Callables
	{
		std::function<void()> bindingsInit;
		std::function<void()> createResources;
		std::function<void()> bindResources;
		std::function<void()> createPipelines;
		std::function<void()> cleanUp;
	};

	

	struct VOIDSTAR_API UniformBufferObject {
		glm::mat4 view;
		glm::mat4 proj;
		alignas(4)float time;
	};



	class Instance;
	class Device;
	class Swapchain;
	class DescriptorSetLayout;
	class CommandPoolManager;
	class VOIDSTAR_API Renderer
	{
		typedef std::unordered_map<std::pair<int, PipelineType>, std::vector < vk::DescriptorSetLayoutBinding>, EnumClassHash>
			Bindings;
		typedef std::unordered_map<std::pair<int, PipelineType>, int, EnumClassHash>
			Sets;

	public:
		void Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window);
		static Renderer* Instance();
		void BeginFrame(Camera& camera, size_t viewportWidth, 
			size_t viewportHeight);
		void Render(float deltaTime,Camera& camera);
		void CompileShader(std::string_view shader);
		void LinkShaders(ProgramHandle handle, uint8_t shaderAmount);
		
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
		void SetCallables(Callables functions) { m_UserFunctions = functions; };
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

	
		Pipeline* GetPipeline(std::string_view pipeline)
		{
			return m_Pipelines.at(pipeline.data()).get();
		}
		void AddPipeline(std::string_view name,UPtr<Pipeline> pipeline)
		{
			m_Pipelines[name.data()] = std::move(pipeline);
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
		TracyVkCtx GetTracyCtx()
		{
			return m_TracyContext;
		}
		CommandBuffer& GetTracyCmd()
		{
			return m_TracyCommandBuffer;
		}
		void UpdateUniformBuffer(const glm::mat4& proj, Camera& camera);
	private:
		void CreateInstance();
		void RecreateSwapchain();
		void AllocateSets();
		void CreateLayouts();
		void CleanUpLayouts();
	private:
		Voidstar::Instance* m_Instance;
		Device* m_Device;
		int m_ViewportWidth, m_ViewportHeight;
		size_t m_CurrentFrame = 0;

		AttachmentManager m_AttachmentManager;
		ShaderCompiler m_Compiler;

		std::vector<CommandBuffer> m_RenderCommandBuffer,
			m_TransferCommandBuffer, m_ComputeCommandBuffer;
		vk::CommandPool m_FrameCommandPool;
		UPtr<IndexBuffer> m_QuadIndexBuffer;
		UPtr<Buffer> m_QuadBuffer{ nullptr };

		UPtr<IndexBuffer> m_SphereIndexBuffer;
		UPtr<Buffer> m_SphereBuffer{ nullptr };
		TracyVkCtx m_TracyContext;


		SPtr<DescriptorPool> m_UniversalPool;
		// can be in one buffer?
		CommandBuffer m_TracyCommandBuffer;
		vk::CommandPool m_TracyCommandPool;

		UPtr<CommandPoolManager> m_CommandPoolManager;
		std::vector<void*> uniformBuffersMapped;

		std::vector<Semaphore> m_ImageAvailableSemaphore;
		std::vector<Semaphore> m_RenderFinishedSemaphore;
		std::vector<Fence> m_InFlightFence;

		std::vector<Semaphore> m_ComputeFinishedSemaphores;
		std::vector<Fence> m_ComputeInFlightFences;

		SPtr<Window> m_Window;

		vk::PolygonMode m_PolygoneMode = vk::PolygonMode::eFill;



		void* m_NoiseDataPtr;

		bool m_IsResized, m_IsNewParametrs, m_IsPolygon;


		// int is number of set, int is a type of pipeline render or compute
		Bindings m_Bindings;
		std::unordered_map<std::pair<int, PipelineType>, DescriptorSetLayout*, EnumClassHash> m_Layout;
		std::unordered_map<std::pair<int, PipelineType>, std::variant<vk::DescriptorSet, std::vector<vk::DescriptorSet> >, EnumClassHash> m_Sets;
		Sets m_SetsAmount;
		Callables m_UserFunctions;
		std::vector<UPtr<RenderPassGraph>> m_Graphs;
		std::unordered_map<std::string,std::vector<Drawable>> m_Drawables;
		std::unordered_map<std::string,std::vector<Drawable>> m_StaticDrawables;

		std::unordered_map<std::string, UPtr<Pipeline>> m_Pipelines;
	
	};

}

