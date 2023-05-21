#pragma once
#include "vulkan/vulkan.hpp"
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
		static Instance* Create(InstanceInfo& info);
		vk::Instance& GetInstance() { return m_Instance; }
	private:
		vk::Instance m_Instance = VK_NULL_HANDLE;
	};
}