#pragma once
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	class VOIDSTAR_API DescriptorSetLayout
	{
	public:
		static DescriptorSetLayout* Create(std::vector<vk::DescriptorSetLayoutBinding>& bindings);
		inline vk::DescriptorSetLayout GetLayout() const  { return m_Layout; }
	private:
		vk::DescriptorSetLayout m_Layout;
	};
}