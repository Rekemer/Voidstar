#include"Prereq.h"
#include"Instance.h"
#include "glfw3.h"
#include "../Log.h"
#if 0
#define BEST_PRACTISES
#else 

#endif // 1

namespace Voidstar
{
	Instance* Instance::Create(InstanceInfo& info)
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

		VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
		VkValidationFeaturesEXT features = {};
		features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		features.enabledValidationFeatureCount = 1;
		features.pEnabledValidationFeatures = enables;

		std::vector<const char*> layers;

		layers.push_back("VK_LAYER_KHRONOS_validation");




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
			auto instance = new Instance();
			instance->m_Instance = vkInstance;
			return instance;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create Instance!");

		}

	}
}