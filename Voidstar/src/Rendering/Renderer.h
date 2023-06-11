#pragma once
#include "vulkan/vulkan.hpp"
#include "../Prereq.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include"Queue.h"
#include"../Types.h"
namespace Voidstar
{
	class Window;
	class Surface;
	class SwapChainSupportDetails;
	class Buffer;
	class Camera;
	class Application;



	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

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
	class DescriptorSetLayout;
	class Renderer
	{
	public:
		Renderer(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app );
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
		void UpdateUniformBuffer(uint32_t imageIndex);

	
	private:
		size_t m_ViewportWidth, m_ViewportHeight;
		Instance* m_Instance;
		Device* m_Device;
		Swapchain* m_Swapchain;
		Buffer* m_Buffer;
		Buffer* m_IndexBuffer;
		Application* m_App;
		
		// can be in one buffer?
		std::vector<Buffer*> m_UniformBuffers;
		std::vector<void*> uniformBuffersMapped;
		
		DescriptorSetLayout* m_DescriptorSetLayout;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		vk::DescriptorPool m_DescriptorPool;
		std::vector<vk::DescriptorSet> m_DescriptorSets;

		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		
	

		

		vk::RenderPass m_RenderPass;
		vk::PipelineLayout m_PipelineLayout;
		vk::Pipeline m_Pipeline;
		UPtr<Queue> m_GraphicsQueue;


		vk::Semaphore m_ImageAvailableSemaphore;
		vk::Semaphore m_RenderFinishedSemaphore;
		vk::Fence	m_InFlightFence;

		SPtr<Window> m_Window;


		
	};

}

