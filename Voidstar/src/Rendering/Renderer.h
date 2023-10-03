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

	
	struct  InstanceData {
		
		alignas(16)float edges [4] = {1.f,1.f,1.f,1.f};
		alignas(16)glm::vec3 pos;
		alignas(4)float scale;
		InstanceData(const glm::vec3& position, float scaling, uint32_t textureIndex)
			: pos(position), scale(scaling) {}
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
	struct NoiseData
	{
		// terrain
		float frequence = 5.f;
		float amplitude = 0.3f;
		float octaves = 3;
		float multipler = 8.f;
		float exponent = 4.f;
		float scale = 3.f;
		// water
		float normalStrength =0.4f ;
		float waterScale = 10.f;
		float waterDepth = 0.3f;
		float waterStrength=0.3f;
		alignas(16)glm::vec3 deepWaterColor = {14/255.f,129/255.f,184/255.f};
		alignas(16)glm::vec3 shallowWaterColor = { 118/255.f,196/255.f,216/255.f };
		// clouds
		alignas(4)float cellAmountA =1.f;
		alignas(4)float cellAmountB = 2.f;
		alignas(4)float cellAmountC = 5.f;
		alignas(4)float persistence;

		float cloudScale = 2;
		float cloudSpeed = 5;
		float cloudScaleMinus = 2.f;
		float cellAmountMinus = 2.f; 

		float numCellsALowRes = 1.f;
		float numCellsBLowRes = 2.f;
		float numCellsCLowRes = 5.f;
		float persistenceLowRes;
	};

	//2500
	#define SIZE_CLOUDS  100
	struct CloudParams
	{
		float densityOffset = 0.5;
		float densityMult = 0.5;
		alignas(16)glm::vec4 weights = {1,1,1,1};
		alignas(16)glm::vec3 lightDir = glm::normalize(glm::vec3(0.5, -1, 0));
		alignas(16)glm::vec3 lightPos = { 0, 124, 4 };
		alignas(16)glm::vec3 cloudPos = { 0, 120, 4 };
		alignas(16)glm::vec3 boxScale = { SIZE_CLOUDS, SIZE_CLOUDS /2.f,SIZE_CLOUDS };
		float lightAbsorption = 1.f;
		float aHg = 0.9f;
		float aHg2 = 0.9f;
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
		void Render(float deltaTime);
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
		const Swapchain& GetSwapchain() { return *m_Swapchain; }
		const DescriptorSetLayout* GetSetLayout(int handle, PipelineType type)
		{
			return m_Layout[{handle, type}];
		}
		std::pair<float, float> GetViewportSize() const { return { m_ViewportWidth,m_ViewportHeight }; }
		vk::PolygonMode GetPolygonMode() const { return m_PolygoneMode; }
	private:
		void CreateInstance();
		void CreateDebugMessenger();
		void CreateSurface();
		void CreateDevice();
		void CreatePipeline();
		void CreateFramebuffers();
		void CreateSyncObjects();
		void CreateMSAAFrame();
		void RecordCommandBuffer(uint32_t imageIndex, vk::RenderPass& renderPass, vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances);
		void UpdateUniformBuffer(uint32_t imageIndex);
		void RecreateSwapchain();
		void Shutdown();
		void AllocateSets();
		void CreateLayouts();
		void CleanUpLayouts();
		

		
	private:
		Voidstar::Instance* m_Instance;
		int m_ViewportWidth, m_ViewportHeight;
		Device* m_Device;
		UPtr<Swapchain> m_Swapchain;
		UPtr<Buffer> m_ModelBuffer{nullptr};
		UPtr<IndexBuffer> m_IndexBuffer;
		
		Application* m_App;
		
		UPtr<Buffer> m_InstancedDataBuffer;

		SPtr<Image> m_Image;
		SPtr<Image> m_ImageSelected;



		

		SPtr<DescriptorPool> m_UniversalPool;
		vk::Image m_MsaaImage;
		vk::DeviceMemory m_MsaaImageMemory;
		vk::ImageView m_MsaaImageView;

		int m_BaseDesc = 0;
		int m_TexDesc = 0;
		int m_Compute = 0;
		vk::DescriptorSet m_DescriptorSetClouds;
		vk::DescriptorSet m_DescriptorSetSky;
		std::vector<vk::DescriptorSet> m_DescriptorSets;
		vk::DescriptorSet m_DescriptorSetTex;
		vk::DescriptorSet m_DescriptorSetSelected;

		// can be in one buffer?
		std::vector<Buffer*> m_UniformBuffers;
		std::vector<CommandBuffer> m_RenderCommandBuffer, m_TransferCommandBuffer, m_ComputeCommandBuffer;
		CommandBuffer m_TracyCommandBuffer;
		vk::CommandPool m_FrameCommandPool;

		std::vector<void*> uniformBuffersMapped;
		


		UPtr<Buffer> m_ShaderStorageBuffer;
		vk::CommandPool m_TracyCommandPool;

		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		
	

		UPtr<Pipeline> m_TerrainPipeline;
		vk::RenderPass m_RenderPass;


		UPtr<Pipeline> m_ComputePipeline;
		size_t nextPoint = 0;


		UPtr<CommandPoolManager> m_CommandPoolManager;
		

		vk::Semaphore m_ImageAvailableSemaphore;
		vk::Semaphore m_RenderFinishedSemaphore;
		vk::Fence	m_InFlightFence;

		std::vector<vk::Semaphore> m_ComputeFinishedSemaphores;
		std::vector<vk::Fence> m_ComputeInFlightFences;

		SPtr<Window> m_Window;

		vk::PolygonMode m_PolygoneMode = vk::PolygonMode::eFill;


		std::vector<glm::vec2> m_ClickPoints;
		void* m_InstancedPtr;

		ImGuiData imguiData;
		NoiseData noiseData;
		void* m_NoiseDataPtr;

		bool m_IsResized, m_IsNewParametrs, m_IsPolygon;


		vk::RenderPass m_ZPass;

		CloudParams cloudParams;
		void* m_CloudPtr;
		UPtr<Buffer> m_CloudBuffer;

		// int is number of set, int is a type of pipeline render or compute
		Bindings m_Bindings;
		std::unordered_map<std::pair<int, PipelineType>, DescriptorSetLayout*, EnumClassHash> m_Layout;
		std::unordered_map<std::pair<int, PipelineType>, std::variant<vk::DescriptorSet, std::vector<vk::DescriptorSet> >, EnumClassHash> m_Sets;
		Sets m_SetsAmount;

	
	};

}

