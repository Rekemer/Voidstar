#pragma once
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	struct DescrriptorSetLayoutSpec
	{
		vk::DescriptorType type;
		vk::ShaderStageFlags stages;
	};
	class DescriptorSetLayout
	{
	public:
		static DescriptorSetLayout* Create(std::vector<vk::DescriptorSetLayoutBinding>& bindings);
		inline vk::DescriptorSetLayout& GetLayout() { return m_Layout; }
	private:
		vk::DescriptorSetLayout m_Layout;
	};
}