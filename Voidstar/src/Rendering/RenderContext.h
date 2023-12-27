#pragma once
#include"Prereq.h"
#include"vulkan/vulkan.hpp"

namespace Voidstar
{
	class Device;
	class CommandBuffer;
	struct Attachment;
	struct Swapchain;
	class Window;
	class InstanceInfo;
	class Instance;
	class Image;
	class SwapChainSupportDetails;
	class RenderContext
	{
	public:
		static Device* GetDevice();
		static Instance* GetInstance() { return m_Instance.get(); };
		static vk::SurfaceKHR* GetSurface() { return m_Surface.get(); };
		static void SetDevice(Device* device);
		static void RecreateSwapchain(SwapChainSupportDetails& attachment);
		static void CreateSwapchain(vk::Format format, size_t width, size_t height,
			vk::PresentModeKHR presentMode,
			vk::ColorSpaceKHR colorSpace);
		static void CreateSurface(Window* window);
		static void CreateInstance(InstanceInfo& window);
		static void CreateDevice();
		static void Shutdown();
		static size_t GetFrameAmount();
		static std::vector<std::shared_ptr<Image>> GetFrames();
	private:
		static void CreateSwapchain(SwapChainSupportDetails& attachment);
		static Device* m_Device;
		static UPtr<Swapchain> m_Swapchain;
		static UPtr<Instance> m_Instance;
		static UPtr <vk::SurfaceKHR> m_Surface;
		static size_t m_FrameAmount;
	};
}