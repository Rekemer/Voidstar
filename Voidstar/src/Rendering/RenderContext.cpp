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
namespace Voidstar
{
	Device* RenderContext::m_Device;
	UPtr<Swapchain> RenderContext::m_Swapchain;
	UPtr <vk::SurfaceKHR> RenderContext::m_Surface;
	UPtr<Instance> RenderContext::m_Instance;

	Device* RenderContext::GetDevice()
	{
		return m_Device;
	}
	void RenderContext::SetDevice(Device* device)
	{
		m_Device = device;
	}
	void RenderContext::CreateSwapchain(SwapChainSupportDetails& details)
	{
		assert(m_Swapchain == nullptr);
		m_Swapchain = Swapchain::Create(details);
	}
	void RenderContext::RecreateSwapchain(SwapChainSupportDetails& details)
	{
		assert(m_Swapchain != nullptr);
		m_Swapchain->CleanUp();

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
	void RenderContext::Shutdown()
	{
		m_Device->GetDevice().destroy();
		m_Instance->DestroyInstance(*m_Surface);
		
	}
}