#include "Prereq.h"
#include "Device.h"
#include "Instance.h"
#include "Image.h"
#include "Buffer.h"
#include <set>
#include "../Log.h"
namespace Voidstar
{
	Device* Device::Create(Instance* instance, vk::SurfaceKHR& surface)
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
				device->m_MsaaSamples = device->GetMaxUsableSampleCount();
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

		

			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics && (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
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

		deviceFeatures.fillModeNonSolid = true;
		deviceFeatures.tessellationShader = true;
		//deviceFeatures.sampleRateShading = true;
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

		// for read only optimal image layout of attachment 
		vk::PhysicalDeviceSynchronization2Features sync2;
		sync2.synchronization2 = true;
		deviceInfo.pNext = &sync2;
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
	vk::SampleCountFlagBits Device::GetMaxUsableSampleCount()
	{
		
		vk::PhysicalDeviceProperties physicalDeviceProperties = m_PhysicalDevice.getProperties();

		vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
			physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
		if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
		if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
		if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
		if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
		if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }
		return vk::SampleCountFlagBits::e1;

	}
	uint32_t Device::FindMemoryTypeIndex( uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties)
	{
		/*
		* // Provided by VK_VERSION_1_0
		typedef struct VkPhysicalDeviceMemoryProperties {
			uint32_t        memoryTypeCount;
			VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
			uint32_t        memoryHeapCount;
			VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
		} VkPhysicalDeviceMemoryProperties;
		*/
		vk::PhysicalDeviceMemoryProperties memoryProperties = m_PhysicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

			//bit i of supportedMemoryIndices is set if that memory type is supported by the device
			bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

			//propertyFlags holds all the memory properties supported by this memory type
			bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

			if (supported && sufficient) {
				return i;
			}
		}

		return 0;
	}
	void Device::UpdateDescriptorSet(vk::DescriptorSet dscSet, int binding, int descriptorCount, Image& image, vk::ImageLayout layout, vk::DescriptorType descType)
	{
		vk::DescriptorImageInfo imageDescriptor;
		imageDescriptor.imageLayout = layout;
		imageDescriptor.imageView = image.m_ImageView;
		imageDescriptor.sampler = image.m_Sampler;

		vk::WriteDescriptorSet descriptorWrite;
		descriptorWrite.dstSet = dscSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = descType;
		descriptorWrite.descriptorCount = descriptorCount;
		descriptorWrite.pImageInfo = &imageDescriptor;
		m_Device.updateDescriptorSets(descriptorWrite, nullptr);
	}

	// to have an array of textures for example
	void Device::UpdateDescriptorSet(vk::DescriptorSet dscSet, int binding, std::vector<vk::DescriptorImageInfo> images, vk::DescriptorType descType)
	{


		vk::WriteDescriptorSet descriptorWrite;
		descriptorWrite.dstSet = dscSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = descType;
		descriptorWrite.descriptorCount = images.size();
		descriptorWrite.pImageInfo = images.data();
		m_Device.updateDescriptorSets(descriptorWrite, nullptr);

		


	}

	void Device::UpdateDescriptorSet(vk::DescriptorSet dscSet, int binding, int descriptorCount, vk::DescriptorImageInfo& imageInfo, vk::DescriptorType type)
	{
		vk::WriteDescriptorSet descriptorWrite;
		descriptorWrite.dstSet = dscSet;
		descriptorWrite.descriptorType = type;
		descriptorWrite.descriptorCount = descriptorCount;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.pImageInfo = &imageInfo;
		m_Device.updateDescriptorSets(descriptorWrite, nullptr);

	}
	
	void Device::UpdateDescriptorSet(vk::DescriptorSet dscSet, int binding, int descriptorCount, Buffer& buffer, vk::DescriptorType type)
	{
		vk::WriteDescriptorSet writeInfo;
		vk::DescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer.GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = buffer.m_Size;

		writeInfo.dstSet = dscSet;
		writeInfo.dstBinding = binding;
		writeInfo.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
		writeInfo.descriptorCount = descriptorCount;
		writeInfo.descriptorType = type;
		writeInfo.pBufferInfo = &bufferInfo;

		m_Device.updateDescriptorSets(writeInfo, nullptr);
	}
	
}
