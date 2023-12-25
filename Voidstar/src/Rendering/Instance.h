#pragma once
#include "vulkan/vulkan.hpp"
#include "Types.h"
namespace Voidstar
{
	struct InstanceInfo
	{
		vk::ApplicationInfo vkInfo;
		uint32_t extensionCount;
		std::vector <const char*> extensions;
		uint32_t layersCount;
		std::vector<const char*> layers;
	};

	class Instance
	{
	public:
		static UPtr<Instance> Create(InstanceInfo& info);
		void CreateDebugMessenger();
		void DestroyInstance(vk::SurfaceKHR& surface);
		vk::Instance& GetInstance() { return m_Instance; }
	private:
		vk::Instance m_Instance = VK_NULL_HANDLE;
		//debug callback
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		//dynamic instance dispatcher
		vk::DispatchLoaderDynamic m_Dldi;
	};
}