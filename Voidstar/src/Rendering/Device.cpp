#include "Prereq.h"
#include "Device.h"
#include "Instance.h"
#include <set>
#include "../Log.h"
namespace Voidstar
{
	Device* Device::Create(Instance* instance, vk::SurfaceKHR surface)
	{
		std::vector<vk::PhysicalDevice> availableDevices = instance->GetInstance().enumeratePhysicalDevices();

		Device* device = new Device();

		/*
		* check if a suitable device can be found
		*/
		const std::vector<const char*> requestedExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		for (vk::PhysicalDevice physicalDevice : availableDevices)
		{
			std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());
			for (vk::ExtensionProperties& extension : physicalDevice.enumerateDeviceExtensionProperties()) {



				//remove this from the list of required extensions (set checks for equality automatically)
				requiredExtensions.erase(extension.extensionName);
			}

			if (requiredExtensions.empty())
			{
				device->m_PhysicalDevice = physicalDevice;
				auto name = physicalDevice.getProperties().deviceName;
				Log::GetLog()->info("PhysicalDevice is found: {0} ", name);


				break;
			}
		}


		




		std::vector<vk::QueueFamilyProperties> queueFamilies = device->m_PhysicalDevice.getQueueFamilyProperties();

		uint32_t graphicFamily = -1;
		uint32_t presentFamily = -1;

		int i = 0;
		for (vk::QueueFamilyProperties queueFamily : queueFamilies) {

		

			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				graphicFamily = i;

			}


			if (device->m_PhysicalDevice.getSurfaceSupportKHR(i, surface)) {
				presentFamily = i;


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

		device->m_GraphicFamily = graphicFamily;
		device->m_PresentFamily = presentFamily;


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
			device->m_Device = device->m_PhysicalDevice.createDevice(deviceInfo);

			Log::GetLog()->info("GPU has been successfully abstracted!");

		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error("Device creation failed!");

		}



		device->m_GraphicsQueue = device->m_Device.getQueue(graphicFamily, 0);
		device->m_PresentQueue = device->m_Device.getQueue(presentFamily, 0);
		return device;
	}
}
