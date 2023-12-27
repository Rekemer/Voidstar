#include"Prereq.h"
#include"RenderContext.h"
#include"Device.h"
#include"CommandBuffer.h"
#include"Swapchain.h"
#include"glfw3.h"
#include"../Window.h"
#include"Instance.h"
#include"../Log.h"
#include"SupportStruct.h"
#include"AttachmentSpec.h"
#include"Swapchain.h"
namespace Voidstar
{
	Device* RenderContext::m_Device;
	UPtr<Swapchain> RenderContext::m_Swapchain;
	UPtr <vk::SurfaceKHR> RenderContext::m_Surface;
	UPtr<Instance> RenderContext::m_Instance;
	size_t RenderContext::m_FrameAmount;

	Device* RenderContext::GetDevice()
	{
		return m_Device;
	}
	void RenderContext::SetDevice(Device* device)
	{
		m_Device = device;
	}
	 size_t RenderContext::GetFrameAmount() { return m_Swapchain->GetFrameAmount(); };
	void RenderContext::CreateSwapchain(SwapChainSupportDetails& details)
	{
		assert(m_Swapchain == nullptr);
		m_Swapchain = Swapchain::Create(details);
	}
	void RenderContext::RecreateSwapchain(SwapChainSupportDetails& details)
	{
		assert(m_Swapchain != nullptr);
		m_Swapchain->CleanUp();
		m_Swapchain = Swapchain::Create(details);
	}
	void RenderContext::CreateSwapchain(vk::Format format, size_t width, size_t height, vk::PresentModeKHR presentMode, vk::ColorSpaceKHR colorSpace)
	{
		SwapchainSpec resolveSpec;
		resolveSpec.Specs.width = width;
		resolveSpec.Specs.height = height;
		resolveSpec.Specs.usage = vk::ImageUsageFlagBits::eColorAttachment;
		resolveSpec.ColorSpace = colorSpace;
		resolveSpec.PresentMode = presentMode;
		resolveSpec.Specs.format = format;
		resolveSpec.Amount = 3;

		auto& swapchainSpec = static_cast<SwapchainSpec&>(resolveSpec);
		SwapChainSupportDetails support;
		auto device = RenderContext::GetDevice();
		auto surface = RenderContext::GetSurface();
		support.AvailableCapabilities = device->GetDevicePhys().getSurfaceCapabilitiesKHR(*surface);
		support.AvailablePresentModes = device->GetDevicePhys().getSurfacePresentModesKHR(*surface);
		support.AvailableFormats = device->GetDevicePhys().getSurfaceFormatsKHR(*surface);
		support.ViewportWidth = swapchainSpec.Specs.width;
		support.ViewportHeight = swapchainSpec.Specs.height;
		support.PresentMode = swapchainSpec.PresentMode;
		support.Format = swapchainSpec.Specs.format;
		support.ColorSpace = swapchainSpec.ColorSpace;
		support.Usage = swapchainSpec.Specs.usage;
		support.FrameAmount = swapchainSpec.Amount;
		RenderContext::CreateSwapchain(support);
	}
	void RenderContext::CreateSurface(Window* window)
	{
		VkSurfaceKHR c_style_surface;
		auto rawWindow = window->GetRaw();
		if (glfwCreateWindowSurface(m_Instance->GetInstance(), rawWindow, nullptr, &c_style_surface) != VK_SUCCESS)
		{

			std::cout << "Failed to abstract glfw surface for Vulkan\n";
			Log::GetLog()->error("Failed to abstract glfw surface for Vulkan");
		}
		else
		{
			Log::GetLog()->info("Successfully abstracted glfw surface for Vulkan");

		}
		//copy constructor converts to hpp convention
		m_Surface = CreateUPtr<vk::SurfaceKHR>(c_style_surface);
	}
	void RenderContext::CreateInstance(InstanceInfo& info)
	{
		m_Instance = Instance::Create(info);
		m_Instance->CreateDebugMessenger();
	}
	void RenderContext::CreateDevice()
	{
		m_Device = Device::Create(m_Instance.get(), *m_Surface);
	}
	std::vector<std::shared_ptr<Image>> RenderContext::GetFrames() { return m_Swapchain->GetImages(); }
	void RenderContext::Shutdown()
	{
		m_Device->GetDevice().destroy();
		m_Instance->DestroyInstance(*m_Surface);
		
	}
}