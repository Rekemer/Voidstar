#include"Prereq.h"
#include"Instance.h"
#include "glfw3.h"
#include "../Log.h"
#if 0
#define BEST_PRACTISES
#else 
#
#define VALIDATION 0 
#endif // 1

namespace Voidstar
{
	UPtr<Instance> Instance::Create(InstanceInfo& info)
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
		std::vector<const char*> layers;


		for (auto i = 0; i < glfwExtensionCount; i++) {
			extensions.emplace_back(glfwExtensions[i]);
		}
#if VALIDATION

		extensions.push_back("VK_EXT_debug_utils");
		layers.push_back("VK_LAYER_KHRONOS_validation");
#endif // 0


		VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
		VkValidationFeaturesEXT features = {};
		features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		features.enabledValidationFeatureCount = 1;
		features.pEnabledValidationFeatures = enables;






		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appInfo,
			layers.size(), layers.data(), // enabled layers
			extensions.size(), extensions.data() // enabled extensions
		);

#ifdef BEST_PRACTISES

		createInfo.pNext = &features;
#endif // BEST_PRACTISES
		try
		{

			auto vkInstance = vk::createInstance(createInfo);
			auto instance = CreateUPtr<Instance>();
			instance->m_Instance = vkInstance;
			return instance;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create Instance!\n {0} ",err.what());

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
	void Instance::CreateDebugMessenger()
	{
#if VALIDATION

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
#endif 

	}
	void Instance::DestroyInstance(vk::SurfaceKHR& surface)
	{
		m_Instance.destroySurfaceKHR(surface);
#if VALIDATION

		m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMessenger, nullptr, m_Dldi);
#endif 
		m_Instance.destroy();
	}
}