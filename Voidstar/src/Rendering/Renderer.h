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





struct ImGui_ImplVulkanH_Window;
namespace Voidstar
{


	


	inline std::string BASE_SHADER_PATH = "../Shaders/";
	inline std::string BASE_RES_PATH = "res";
	//const std::string SPIRV_COMPILER_PATH = "C:/VulkanSDK/1.3.216.0/Bin/glslc.exe";
	const std::string SPIRV_COMPILER_PATH = "C:/VulkanSDK/1.3.216.0/Bin/glslangvalidator.exe";
	inline std::string BASE_SPIRV_OUTPUT = BASE_SHADER_PATH + "Binary/";

	class Window;
	class Surface;
	class SwapChainSupportDetails;
	class Buffer;
	class Camera;
	class Application;
	class Image;
	class DescriptorPool;
	class Model;
	class Pipeline;
	class Vertex;



	struct Callables
	{
		std::function<void()> bindingsInit;
		std::function<void()> bufferInit;
		std::function<void()> loadTextures;
		std::function<void()> bindResources;
		std::function<void()> createPipelines;
		std::function<void()> createFramebuffer;
		std::function<CommandBuffer(size_t frameIndex, Camera& camera)> submitRenderCommands;
		std::function<void(size_t frameIndex, Camera& camera)> postRenderCommands;
		std::function<void()> cleanUp;
	};

	

	struct UniformBufferObject {
		glm::mat4 view;
		glm::mat4 proj;
		alignas(16)glm::vec4 playerPos;
		alignas(4)float time;
	};


	

	struct GraphicsPipeline
	{
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};
	struct ImGuiData
	{
		VkDescriptorPool g_DescriptorPool;
		VkRenderPass g_RenderPass;
		vk::CommandPool g_CommandPool;
		std::vector<CommandBuffer> g_CommandBuffers;
		std::vector<vk::Framebuffer> g_FrameBuffers;
	};
	


	class Instance;
	class Device;
	class Swapchain;
	class DescriptorSetLayout;
	class CommandPoolManager;
	class Renderer
	{
		typedef std::unordered_map<std::pair<int, PipelineType>, std::vector < vk::DescriptorSetLayoutBinding>, EnumClassHash>
			Bindings;
		typedef std::unordered_map<std::pair<int, PipelineType>, int, EnumClassHash>
			Sets;
	public:
		void Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app );
		void InitImGui( );
		void SetupVulkanWindow(ImGui_ImplVulkanH_Window* g_wd, VkSurfaceKHR surface, int width, int height);
		static Renderer* Instance();
		void Render(float deltaTime,Camera& camera);
		void UserInit();
		CommandPoolManager* GetCommandPoolManager()
		{
			return m_CommandPoolManager.get();
		}

		void RenderImGui(int frameIndex);
		void CleanUpImGui();
		~Renderer();
		Sets& GetSets() { return m_SetsAmount; }
		Bindings& GetBindings() { return m_Bindings; }
		template<typename T>
		const T GetSet(int handle, PipelineType type)
		{
			return std::get<T>(m_Sets[{handle, type}]);;
		}
		 Swapchain& GetSwapchain() { return *m_Swapchain; }
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
		void DrawQuadScreen(vk::CommandBuffer commandBuffer);
		void DrawQuad(vk::CommandBuffer commandBuffer, glm::mat4& world, glm::vec4 color);
		void DrawTxt(vk::CommandBuffer commandBuffer, std::string_view str,glm::vec2 pos, std::map<unsigned char, Character> &Characters);
		void BeginBatch();
		void DrawBatch(vk::CommandBuffer& commandBuffer);
		UPtr<Buffer> m_QuadBufferBatch{ nullptr };
		UPtr<IndexBuffer> m_QuadBufferBatchIndex{ nullptr };
		Vertex * m_BatchQuad;
		Vertex * m_BatchQuadStart;
		int m_QuadIndex= 0;
	private:
		void CreateInstance();
		void CreateDebugMessenger();
		void CreateSurface();
		void CreateDevice();
		void CreatePipeline();
		void CreateSyncObjects();
		void RecordCommandBuffer(uint32_t imageIndex, vk::RenderPass& renderPass, vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances);
		void RecreateSwapchain();
		void AllocateSets();
		void CreateLayouts();
		void CleanUpLayouts();
		void FreeBuffers();

		
	private:
		std::vector <UPtr<Buffer>> m_Buffers;
		Voidstar::Instance* m_Instance;
		int m_ViewportWidth, m_ViewportHeight;
		Device* m_Device;
		UPtr<Swapchain> m_Swapchain;
		
		Application* m_App;
		


		std::vector<CommandBuffer> m_RenderCommandBuffer,
			m_TransferCommandBuffer, m_ComputeCommandBuffer;
		vk::CommandPool m_FrameCommandPool;


		

		UPtr<IndexBuffer> m_QuadIndexBuffer;
		UPtr<Buffer> m_QuadBuffer{ nullptr };

		SPtr<DescriptorPool> m_UniversalPool;


		// can be in one buffer?
		CommandBuffer m_TracyCommandBuffer;

		


		vk::CommandPool m_TracyCommandPool;

		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		
	





		UPtr<CommandPoolManager> m_CommandPoolManager;
		

		vk::Semaphore m_ImageAvailableSemaphore;
		vk::Semaphore m_RenderFinishedSemaphore;
		vk::Fence	m_InFlightFence;

		std::vector<vk::Semaphore> m_ComputeFinishedSemaphores;
		std::vector<vk::Fence> m_ComputeInFlightFences;

		SPtr<Window> m_Window;

		vk::PolygonMode m_PolygoneMode = vk::PolygonMode::eFill;



		ImGuiData imguiData;
		void* m_NoiseDataPtr;

		bool m_IsResized, m_IsNewParametrs, m_IsPolygon;


		// int is number of set, int is a type of pipeline render or compute
		Bindings m_Bindings;
		std::unordered_map<std::pair<int, PipelineType>, DescriptorSetLayout*, EnumClassHash> m_Layout;
		std::unordered_map<std::pair<int, PipelineType>, std::variant<vk::DescriptorSet, std::vector<vk::DescriptorSet> >, EnumClassHash> m_Sets;
		Sets m_SetsAmount;
		Callables m_UserFunctions;
	
	};

}

