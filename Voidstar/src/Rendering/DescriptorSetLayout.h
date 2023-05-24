#pragma once
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	struct DescrriptorSetLayoutSpec
	{
		vk::DescriptorType type;
		vk::ShaderStageFlags flags;
	};
	class DescriptorSetLayout
	{
	public:
		static DescriptorSetLayout* Create(DescrriptorSetLayoutSpec& specs);
		inline vk::DescriptorSetLayout& GetLayout() { return m_Layout; }
	private:
		vk::DescriptorSetLayout m_Layout;
	};
}