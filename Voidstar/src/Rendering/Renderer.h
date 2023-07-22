#pragma once
#include "vulkan/vulkan.hpp"
#include "../Prereq.h"
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
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
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


	struct GraphicsPipelineSpecification
	{
		
		vk::Device device;
		std::string vertexFilepath="";
		std::string fragmentFilepath="";
		std::string geometryShader = "";

		// tesselation shaders
		// control shader
		std::string tessCFilepath = "";
		// evalatuation shader
		std::string tessEFilepath = "";


		std::string computeFilepath = "";



		vk::SampleCountFlagBits samples;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat;
		std::vector<vk::VertexInputBindingDescription> bindingDescription;
		std::vector<vk::VertexInputAttributeDescription>  attributeDescription;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayout;
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
	public:
		void Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app );
		void InitImGui( );
		void SetupVulkanWindow(ImGui_ImplVulkanH_Window* g_wd, VkSurfaceKHR surface, int width, int height);
		static Renderer* Instance();
		void Render();
		CommandPoolManager* GetCommandPoolManager()
		{
			return m_CommandPoolManager.get();
		}

		void SubmitInstanceData(const InstanceData& instance);
		void GenerateTerrain();
		void GenerateTerrain(glm::vec3 tilePos,float depth, float tileWidth, int parentIndex);
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
		GraphicsPipeline CreatePipeline(GraphicsPipelineSpecification& spec, vk::PrimitiveTopology topology);
		void DestroySwapchain();
		void CreateSyncObjects();
		void CreateMSAAFrame();
		
		void RecordCommandBuffer(uint32_t imageIndex, vk::RenderPass& renderPass, vk::Pipeline& pipeline, vk::PipelineLayout& pipelineLayout, int instances);
		void UpdateUniformBuffer(uint32_t imageIndex);
		void RecreateSwapchain();
		void Shutdown();
		void CreateComputePipeline();
	private:
		Voidstar::Instance* m_Instance;
		int m_ViewportWidth, m_ViewportHeight;
		Device* m_Device;
		UPtr<Swapchain> m_Swapchain;
		UPtr<Buffer> m_ModelBuffer{nullptr};
		UPtr<IndexBuffer> m_IndexBuffer;
		UPtr<Buffer> m_SphereBuffer;
		UPtr<IndexBuffer> m_IndexSphereBuffer;
		
		Application* m_App;
		
		UPtr<Buffer> m_InstancedDataBuffer;

		SPtr<Image> m_Image;
		SPtr<Image> m_NoiseImage;
		SPtr<Model> m_Model;


		vk::Image m_MsaaImage;
		vk::DeviceMemory m_MsaaImageMemory;
		vk::ImageView m_MsaaImageView;


		SPtr<DescriptorPool> m_DescriptorPool;

		// can be in one buffer?
		std::vector<Buffer*> m_UniformBuffers;
		std::vector<CommandBuffer> m_RenderCommandBuffer, m_TransferCommandBuffer;
		CommandBuffer m_TracyCommandBuffer;


		std::vector<void*> uniformBuffersMapped;
		
		DescriptorSetLayout* m_DescriptorSetLayout;
		DescriptorSetLayout* m_DescriptorSetLayoutTex;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		std::vector<vk::DescriptorSet> m_DescriptorSets;
		std::vector<UPtr<Buffer>> m_ShaderStorageBuffers;
		vk::CommandPool m_CommandComputePool;
		std::vector<CommandBuffer> m_ComputeCommandBuffer;

		SPtr<DescriptorPool> m_DescriptorPoolTex;
		vk::DescriptorSet m_DescriptorSetTex;
		std::vector<vk::DescriptorSet> m_ComputeDescriptorSets;
		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		
	

		

		vk::Pipeline m_Pipeline;
		vk::RenderPass m_RenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_DebugPipeline;
		vk::PipelineLayout m_DebugPipelineLayout;
		vk::RenderPass m_DebugRenderPass;

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


		vk::BufferMemoryBarrier	m_InstanceBarrier;
		std::vector<InstanceData> m_InstanceData;
		std::vector<vk::DescriptorSet> m_InstanceDescriptorSets;
		void* m_InstancedPtr;
		Quadtree m_QuadTree;

		ImGuiData imguiData;
	
	};

}

