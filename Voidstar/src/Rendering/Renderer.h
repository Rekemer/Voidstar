#pragma once
#include "../Prereq.h"
#include "vulkan/vulkan.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include"CommandBuffer.h"
#include"../Types.h"
#include"QuadTree/QuadTree.h"
struct ImGui_ImplVulkanH_Window;
namespace Voidstar
{
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


	enum class RenderPrimitive
	{
		Plane,
		Cube,
	};
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

	struct Particle {
		glm::vec2 position;
		glm::vec2 velocity;
		glm::vec4 color;

		static vk::VertexInputBindingDescription GetBindingDescription() {
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Particle);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;
			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescriptions() {
			std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[0].offset = offsetof(Particle, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32A32Sfloat;
			attributeDescriptions[1].offset = offsetof(Particle, color);

			return attributeDescriptions;
		}

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

	#define SIZE_CLOUDS  100.f
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
	};
	class Instance;
	class Device;
	class Swapchain;
	class DescriptorSetLayout;
	class CommandPoolManager;
	class Renderer
	{
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

		void SubmitInstanceData(const InstanceData& instance);
		void RenderImGui(int frameIndex);
		void CleanUpImGui();
		~Renderer();
	private:
		void CreateInstance();
		void CreateDebugMessenger();
		void CreateSurface();
		void CreateDevice();
		void CreatePipeline();
		void CreateFramebuffers();
		void DestroySwapchain();
		void CreateSyncObjects();
		void CreateMSAAFrame();
		
		void RecordCommandBuffer(uint32_t imageIndex, vk::RenderPass& renderPass, vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances);
		void UpdateUniformBuffer(uint32_t imageIndex);
		void RecreateSwapchain();
		void Shutdown();
		void CreateComputePipeline();
		void UpdateNoiseTexture();
		void UpdateCloudTexture();
	private:
		Voidstar::Instance* m_Instance;
		int m_ViewportWidth, m_ViewportHeight;
		Device* m_Device;
		UPtr<Swapchain> m_Swapchain;
		UPtr<Buffer> m_ModelBuffer{nullptr};
		UPtr<Buffer> m_CubeBuffer{nullptr};
		UPtr<IndexBuffer> m_IndexBuffer;
		UPtr<IndexBuffer> m_IndexCubeBuffer;
		UPtr<Buffer> m_SphereBuffer;
		UPtr<IndexBuffer> m_IndexSphereBuffer;
		
		Application* m_App;
		
		UPtr<Buffer> m_InstancedDataBuffer;
		UPtr<Buffer> m_PointsData;

		SPtr<Image> m_Image;
		SPtr<Image> m_NoiseImage;

		SPtr<Image> m_AnimatedNoiseImage;
		SPtr<Image> m_3DNoiseTexture;
		SPtr<Image> m_3DNoiseTextureLowRes;
		
		SPtr<Image> m_WaterNormalImage;
		SPtr<Image> m_WaterNormalImage2;


		SPtr<Image> m_SnowTex;
		SPtr<Image>	m_GrassTex;
		SPtr<Image>	m_StoneTex;
		SPtr<Image>	m_Cubemap;
		
		SPtr<Model> m_Model;


		vk::Image m_MsaaImage;
		vk::DeviceMemory m_MsaaImageMemory;
		vk::ImageView m_MsaaImageView;


		SPtr<DescriptorPool> m_DescriptorPool;
		SPtr<DescriptorPool> m_DescriptorPoolClouds;
		DescriptorSetLayout* m_DescriptorSetLayoutClouds;
		vk::DescriptorSet m_DescriptorSetClouds;

		// can be in one buffer?
		std::vector<Buffer*> m_UniformBuffers;
		std::vector<CommandBuffer> m_RenderCommandBuffer, m_TransferCommandBuffer;
		CommandBuffer m_TracyCommandBuffer;
		vk::CommandPool m_FrameCommandPool;

		std::vector<void*> uniformBuffersMapped;
		
		vk::DescriptorSet m_DescriptorSetSky;
		DescriptorSetLayout* m_DescriptorSetLayoutSky;
		SPtr<DescriptorPool> m_DescriptorPoolSky;

		DescriptorSetLayout* m_DescriptorSetLayout;

		DescriptorSetLayout* m_DescriptorSetLayoutTex;
		std::vector<vk::DescriptorSet> m_DescriptorSets;
		std::vector<UPtr<Buffer>> m_ShaderStorageBuffers;
		vk::CommandPool m_TracyCommandPool;
		std::vector<CommandBuffer> m_ComputeCommandBuffer;

		SPtr<DescriptorPool> m_DescriptorPoolTex;
		vk::DescriptorSet m_DescriptorSetTex;
		std::vector<vk::DescriptorSet> m_ComputeDescriptorSets;
		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		
	

		UPtr<Pipeline> m_TerrainPipeline;
		UPtr<Pipeline> m_WaterPipeline;
		UPtr<Pipeline> m_SkyPipeline;
		UPtr<Pipeline> m_RayMarchPipeline;
		// Z Prepass
		UPtr<Pipeline> m_ZTerrainPipeline;
		UPtr<Pipeline> m_ZWaterPipeline;

		
		vk::Pipeline m_DebugPipeline;
		vk::PipelineLayout m_DebugPipelineLayout;
		vk::RenderPass m_DebugRenderPass;
		vk::Pipeline m_ComputePipelineClouds;
		vk::PipelineLayout m_ComputePipelineLayoutClouds;

		vk::Pipeline m_ComputePipeline;
		vk::PipelineLayout m_ComputePipelineLayout;
		vk::DescriptorSet m_DescriptorSetNoise;
		SPtr<DescriptorPool> m_DescriptorPoolNoise;
		DescriptorSetLayout* m_DescriptorSetLayoutNoise;


		UPtr<CommandPoolManager> m_CommandPoolManager;
		

		vk::Semaphore m_ImageAvailableSemaphore;
		vk::Semaphore m_RenderFinishedSemaphore;
		vk::Fence	m_InFlightFence;

		std::vector<vk::Semaphore> m_ComputeFinishedSemaphores;
		std::vector<vk::Fence> m_ComputeInFlightFences;

		SPtr<Window> m_Window;

		RenderPrimitive m_RenderPrimitive = RenderPrimitive::Plane;
		vk::PolygonMode m_PolygoneMode = vk::PolygonMode::eFill;


		//vk::BufferMemoryBarrier	m_InstanceBarrier;
		std::vector<InstanceData> m_InstanceData;
		std::vector<vk::DescriptorSet> m_InstanceDescriptorSets;
		void* m_InstancedPtr;
		Quadtree m_QuadTree;

		ImGuiData imguiData;
		NoiseData noiseData;
		void* m_NoiseDataPtr;
		Buffer* m_NoiseData;

		bool m_IsResized, m_IsNewParametrs, m_IsPolygon;


		vk::RenderPass m_ZPass;

		CloudParams cloudParams;
		void* m_CloudPtr;
		UPtr<Buffer> m_CloudBuffer;
	
	};

}

