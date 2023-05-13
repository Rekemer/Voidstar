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
	};

	class Renderer
	{
	public:
		Renderer(size_t screenWidth, size_t screenHeight, Window* window);
		~Renderer();
	private:
		void CreateInstance();
		void CreateDebugMessenger();
		void CreateSurface();
		void CreateDevice();
		void CreateSwapchain();

		void DestroySwapchain();
		
		
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

		Window* m_Window;
	};

}

