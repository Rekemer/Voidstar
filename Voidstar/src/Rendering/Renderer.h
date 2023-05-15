#pragma once
#include "vulkan/vulkan.hpp"
namespace Voidstar
{
	class Window;
	
	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};
	struct SwapChainFrame {
		vk::Image image;
		vk::ImageView imageView;
		vk::Framebuffer framebuffer;
	};
	struct GraphicsPipelineSpecification
	{
		
		vk::Device device;
		std::string vertexFilepath;
		std::string fragmentFilepath;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainImageFormat;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayout;
	};

	struct GraphicsPipeline
	{
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};

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

		vk::SurfaceFormatKHR GetSurfaceFormat(vk::Format format, vk::ColorSpaceKHR colorSpace,SwapChainSupportDetails& support);
		vk::PresentModeKHR GetPresentMode(vk::PresentModeKHR presentMode, SwapChainSupportDetails& support);
		vk::Extent2D GetSwapchainExtent(uint32_t width, uint32_t height, SwapChainSupportDetails& support);
	private:
		size_t m_ViewportWidth, m_ViewportHeight;
		vk::Instance m_Instance;
	


		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;



		vk::SurfaceKHR m_Surface;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::Device m_Device;
		size_t m_GraphicFamily{ 0 }, m_PresentFamily{ 0 };
		vk::Queue m_GraphicsQueue;
		vk::Queue m_PresentQueue;

		vk::SwapchainKHR m_Swapchain{ nullptr };
		std::vector<SwapChainFrame> m_SwapchainFrames;
		vk::Format m_SwapchainFormat;
		vk::Extent2D m_SwapchainExtent;

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

