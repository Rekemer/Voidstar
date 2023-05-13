#include"Prereq.h"
#include "Renderer.h"
#include <set>
#include "../Window.h"
#include "../Log.h"
#include "glfw3.h"

namespace Voidstar
{
	Renderer::Renderer(size_t screenWidth, size_t screenHeight, Window* window) :
		m_Window{window}, m_ViewportWidth(screenWidth), m_ViewportHeight(screenHeight)
	{
		

		// create instance
		CreateInstance();

		CreateDebugMessenger();

		CreateSurface();

		CreateDevice();

		CreateSwapchain();
	}

	void Renderer::DestroySwapchain()
	{
		for (auto& frame : m_SwapchainFrames) {

			m_Device.destroyImageView(frame.imageView);
			

		}
		// cannot not use detroy image on  presentable image
		m_Device.destroySwapchainKHR(m_Swapchain);

	}

	Renderer::~Renderer()
	{
		m_Device.waitIdle();
		DestroySwapchain();
		m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dldi);
		m_Instance.destroySurfaceKHR(m_Surface);
		m_Device.destroy();
		m_Instance.destroy();
	}
	void Renderer::CreateDevice()
	{
	

		std::vector<vk::PhysicalDevice> availableDevices = m_Instance.enumeratePhysicalDevices();

		

		/*
		* check if a suitable device can be found
		*/
		const std::vector<const char*> requestedExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		for (vk::PhysicalDevice device : availableDevices)
		{	
			std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());
			for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {

				

				//remove this from the list of required extensions (set checks for equality automatically)
				requiredExtensions.erase(extension.extensionName);
			}

			if (requiredExtensions.empty())
			{
				m_PhysicalDevice = device;
				auto name = m_PhysicalDevice.getProperties().deviceName;
				Log::GetLog()->info("PhysicalDevice is found: {0} ", name);
			

				break;
			}
		}

	


		
		

		std::vector<vk::QueueFamilyProperties> queueFamilies = m_PhysicalDevice.getQueueFamilyProperties();

		uint32_t graphicFamily = -1;
		uint32_t presentFamily = -1;

		int i = 0;
		for (vk::QueueFamilyProperties queueFamily : queueFamilies) {

			/*
			* // Provided by VK_VERSION_1_0
				typedef struct VkQueueFamilyProperties {
				VkQueueFlags    queueFlags;
				uint32_t        queueCount;
				uint32_t        timestampValidBits;
				VkExtent3D      minImageTransferGranularity;
				} VkQueueFamilyProperties;
				queueFlags is a bitmask of VkQueueFlagBits indicating capabilities of the queues in this queue family.
				queueCount is the unsigned integer count of queues in this queue family. Each queue family must support
				at least one queue.
				timestampValidBits is the unsigned integer count of meaningful bits in the timestamps written via
				vkCmdWriteTimestamp. The valid range for the count is 36..64 bits, or a value of 0,
				indicating no support for timestamps. Bits outside the valid range are guaranteed to be zeros.
				minImageTransferGranularity is the minimum granularity supported for image transfer
				operations on the queues in this queue family.
			*/

			/*
			* // Provided by VK_VERSION_1_0
				typedef enum VkQueueFlagBits {
				VK_QUEUE_GRAPHICS_BIT = 0x00000001,
				VK_QUEUE_COMPUTE_BIT = 0x00000002,
				VK_QUEUE_TRANSFER_BIT = 0x00000004,
				VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
				} VkQueueFlagBits;
			*/

			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				graphicFamily = i;

			}


			if (m_PhysicalDevice.getSurfaceSupportKHR(i, m_Surface)) {
				presentFamily =  i;

				
			}


			if (graphicFamily != -1 && presentFamily != -1) {
				break;
			}

			i++;
		}


		std::vector<uint32_t> uniqueIndices;

		uniqueIndices.push_back(graphicFamily);

		if (graphicFamily != presentFamily) {
			uniqueIndices.push_back(presentFamily);
		}

		m_GraphicFamily = graphicFamily;
		m_PresentFamily = presentFamily;


		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
		float queuePriority = 1.0f;
		for (uint32_t queueFamilyIndex : uniqueIndices) {
			queueCreateInfo.push_back(
				vk::DeviceQueueCreateInfo(
					vk::DeviceQueueCreateFlags(), queueFamilyIndex, 1, &queuePriority
				)
			);
		}


		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();



		std::vector<const char*> enabledLayers;
		
		enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		
		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			1, queueCreateInfo.data(),
			enabledLayers.size(), enabledLayers.data(),
			deviceExtensions.size(), deviceExtensions.data(),
			&deviceFeatures
		);

		try {
			m_Device = m_PhysicalDevice.createDevice(deviceInfo);
			
			Log::GetLog()->info( "GPU has been successfully abstracted!");
		
		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error( "Device creation failed!");
				
		}

		
		
		m_GraphicsQueue  = m_Device.getQueue(graphicFamily, 0);
		m_PresentQueue  = m_Device.getQueue(presentFamily, 0);
		
		
	}
	
	

	void Renderer::CreateSwapchain()
	{
		SwapChainSupportDetails support;

		support.capabilities = m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface);
	
		support.formats = m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface);
		support.presentModes = m_PhysicalDevice.getSurfacePresentModesKHR(m_Surface);



		vk::SurfaceFormatKHR format = GetSurfaceFormat(vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear, support);

	

		vk::PresentModeKHR presentMode = GetPresentMode(vk::PresentModeKHR::eMailbox, support);

		

		vk::Extent2D extent = GetSwapchainExtent(m_ViewportWidth,m_ViewportHeight, support);

		uint32_t imageCount = std::min(
			support.capabilities.maxImageCount,
			support.capabilities.minImageCount + 1
		);

		/*
		* VULKAN_HPP_CONSTEXPR SwapchainCreateInfoKHR(
	  VULKAN_HPP_NAMESPACE::SwapchainCreateFlagsKHR flags_         = {},
	  VULKAN_HPP_NAMESPACE::SurfaceKHR              surface_       = {},
	  uint32_t                                      minImageCount_ = {},
	  VULKAN_HPP_NAMESPACE::Format                  imageFormat_   = VULKAN_HPP_NAMESPACE::Format::eUndefined,
	  VULKAN_HPP_NAMESPACE::ColorSpaceKHR   imageColorSpace_  = VULKAN_HPP_NAMESPACE::ColorSpaceKHR::eSrgbNonlinear,
	  VULKAN_HPP_NAMESPACE::Extent2D        imageExtent_      = {},
	  uint32_t                              imageArrayLayers_ = {},
	  VULKAN_HPP_NAMESPACE::ImageUsageFlags imageUsage_       = {},
	  VULKAN_HPP_NAMESPACE::SharingMode     imageSharingMode_ = VULKAN_HPP_NAMESPACE::SharingMode::eExclusive,
	  uint32_t                              queueFamilyIndexCount_ = {},
	  const uint32_t *                      pQueueFamilyIndices_   = {},
	  VULKAN_HPP_NAMESPACE::SurfaceTransformFlagBitsKHR preTransform_ =
		VULKAN_HPP_NAMESPACE::SurfaceTransformFlagBitsKHR::eIdentity,
	  VULKAN_HPP_NAMESPACE::CompositeAlphaFlagBitsKHR compositeAlpha_ =
		VULKAN_HPP_NAMESPACE::CompositeAlphaFlagBitsKHR::eOpaque,
	  VULKAN_HPP_NAMESPACE::PresentModeKHR presentMode_  = VULKAN_HPP_NAMESPACE::PresentModeKHR::eImmediate,
	  VULKAN_HPP_NAMESPACE::Bool32         clipped_      = {},
	  VULKAN_HPP_NAMESPACE::SwapchainKHR   oldSwapchain_ = {} ) VULKAN_HPP_NOEXCEPT
		*/
		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(), m_Surface, imageCount, format.format, format.colorSpace,
			extent, 1, vk::ImageUsageFlagBits::eColorAttachment
		);


		
		uint32_t queueFamilyIndices[] = { m_GraphicFamily, m_PresentFamily };

		if (m_PresentFamily != m_GraphicFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = support.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		
		try {
			m_Swapchain = m_Device.createSwapchainKHR(createInfo);
			Log::GetLog()->info("Swapchain created!");
		}
		catch (vk::SystemError err) {
			Log::GetLog()->error("failed to create swap chain!");
			throw std::runtime_error("failed to create swap chain!");
		}


		// images already exists at this point
		auto images = m_Device.getSwapchainImagesKHR(m_Swapchain);
		m_SwapchainFrames.resize(images.size());
		for (size_t i = 0; i < images.size(); ++i) {

			/*
			* ImageViewCreateInfo( VULKAN_HPP_NAMESPACE::ImageViewCreateFlags flags_ = {},
						   VULKAN_HPP_NAMESPACE::Image                image_ = {},
						   VULKAN_HPP_NAMESPACE::ImageViewType    viewType_  = VULKAN_HPP_NAMESPACE::ImageViewType::e1D,
						   VULKAN_HPP_NAMESPACE::Format           format_    = VULKAN_HPP_NAMESPACE::Format::eUndefined,
						   VULKAN_HPP_NAMESPACE::ComponentMapping components_            = {},
						   VULKAN_HPP_NAMESPACE::ImageSubresourceRange subresourceRange_ = {} ) VULKAN_HPP_NOEXCEPT
				: flags( flags_ )
				, image( image_ )
				, viewType( viewType_ )
				, format( format_ )
				, components( components_ )
				, subresourceRange( subresourceRange_ )
			*/

			vk::ImageViewCreateInfo createInfo = {};
			createInfo.image = images[i];
			createInfo.viewType = vk::ImageViewType::e2D;
			createInfo.format = format.format;
			createInfo.components.r = vk::ComponentSwizzle::eIdentity;
			createInfo.components.g = vk::ComponentSwizzle::eIdentity;
			createInfo.components.b = vk::ComponentSwizzle::eIdentity;
			createInfo.components.a = vk::ComponentSwizzle::eIdentity;
			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			m_SwapchainFrames[i].image = images[i];
			m_SwapchainFrames[i].imageView = m_Device.createImageView(createInfo);
		}

		m_SwapchainFormat = format.format;
		m_SwapchainExtent = extent;
	}

	vk::SurfaceFormatKHR Renderer::GetSurfaceFormat(vk::Format format, vk::ColorSpaceKHR colorSpace, SwapChainSupportDetails& support)
	{
		for (vk::SurfaceFormatKHR formatSupport : support.formats)
		{
			if (formatSupport.format == format
				&& formatSupport.colorSpace == colorSpace) {
				return formatSupport;
			}
		}

		return support.formats[0];
	}

	vk::PresentModeKHR Renderer::GetPresentMode(vk::PresentModeKHR presentMode, SwapChainSupportDetails& support)
	{
		for (vk::PresentModeKHR presentModeSupport : support.presentModes)
		{
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				presentMode = presentModeSupport;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}
	vk::Extent2D Renderer::GetSwapchainExtent(uint32_t width, uint32_t height, SwapChainSupportDetails& support) {

		auto capabilities = support.capabilities;
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			vk::Extent2D extent = { width, height };

			extent.width = std::min(
				capabilities.maxImageExtent.width,
				std::max(capabilities.minImageExtent.width, extent.width)
			);

			extent.height = std::min(
				capabilities.maxImageExtent.height,
				std::max(capabilities.minImageExtent.height, extent.height)
			);

			return extent;
		}
	}
	void Renderer::CreateSurface()
	{
		VkSurfaceKHR c_style_surface;
		auto rawWindow = m_Window->GetRaw();
		if (glfwCreateWindowSurface(m_Instance,rawWindow, nullptr, &c_style_surface) != VK_SUCCESS)
		{
		
			std::cout << "Failed to abstract glfw surface for Vulkan\n";
			Log::GetLog()->error("Failed to abstract glfw surface for Vulkan");
		}
		else 
		{
			Log::GetLog()->info("Successfully abstracted glfw surface for Vulkan");
			
		}
		//copy constructor converts to hpp convention
		m_Surface = c_style_surface;
	}
	void Renderer::CreateInstance()
	{
		uint32_t version;
		vkEnumerateInstanceVersion(&version);

		auto appInfo = vk::ApplicationInfo
		{
			"Voidstar app",
			version,
			"Voidstar",
			version,
			version,
		};
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


		std::vector<const char*> extensions;


		for (auto i = 0; i < glfwExtensionCount; i++) {
			extensions.emplace_back(glfwExtensions[i]);
		}
		
		extensions.push_back("VK_EXT_debug_utils");
		
		

		std::vector<const char*> layers;

		layers.push_back("VK_LAYER_KHRONOS_validation");

		

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			layers.size(), layers.data(), // enabled layers
			extensions.size(), extensions.data() // enabled extensions
		);


		try
		{
			
			m_Instance =  vk::createInstance(createInfo);
			Log::GetLog()->info("Instance is created!");
		}
		catch (vk::SystemError err) 
		{
				Log::GetLog()->error("Failed to create Instance!");
				
		}
	
	}

	inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void Renderer::CreateDebugMessenger()
	{
		m_Dldi = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);

	
		vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
			vk::DebugUtilsMessengerCreateFlagsEXT(),
			/*vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |*/ vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			debugCallback,
			nullptr
		);
		
		try
		{
			m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(createInfo, nullptr, m_Dldi);
			Log::GetLog()->info("m_DebugMessenger is created!");
		}
		catch (vk::SystemError& err)
		{
			Log::GetLog()->error("Falied to create m_DebugMessenger!");
		}
		

		
	}
	
}
