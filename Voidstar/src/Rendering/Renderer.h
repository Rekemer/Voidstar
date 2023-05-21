#pragma once
#include "vulkan/vulkan.hpp"
#include "../Prereq.h"
namespace Voidstar
{
	class Window;
	class Surface;
	class SwapChainSupportDetails;
	class VertexBuffer;


	struct GraphicsPipelineSpecification
	{
		
		vk::Device device;
		std::string vertexFilepath;
		std::string fragmentFilepath;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat;
		vk::VertexInputBindingDescription bindingDescription;
		std::array<vk::VertexInputAttributeDescription, 2>  attributeDescription;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayout;
	};

	struct GraphicsPipeline
	{
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};
	class Instance;
	class Device;
	class Swapchain;
	class Renderer
	{
	public:
		Renderer(size_t screenWidth, size_t screenHeight, Window* window);
		void Render();
		~Renderer();
	private:
		void CreateInstance();
		void CreateDebugMessenger();
		void CreateSurface();
		void CreateDevice();
		void CreateSwapchain();
		void CreatePipeline();
		void CreateFramebuffers();
		GraphicsPipeline CreatePipeline(GraphicsPipelineSpecification& spec);
		void DestroySwapchain();
		void CreateCommandPool();
		void CreateCommandBuffer();
		void CreateSyncObjects();
		
		
		void RecordCommandBuffer(uint32_t imageIndex);

	
	private:
		size_t m_ViewportWidth, m_ViewportHeight;
		Instance* m_Instance;
		Device* m_Device;
		Swapchain* m_Swapchain;
		VertexBuffer* m_Buffer;

		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		
	

		

		vk::RenderPass m_RenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_Pipeline;
		vk::CommandPool m_CommandPool;
		vk::CommandBuffer m_CommandBuffer;


		vk::Semaphore m_ImageAvailableSemaphore;
		vk::Semaphore m_RenderFinishedSemaphore;
		vk::Fence	m_InFlightFence;

		Window* m_Window;
	};

}

