#pragma once
#include"vulkan/vulkan.hpp"
namespace Voidstar
{
	struct VOIDSTAR_API DescrriptorSetLayoutSpec
	{
		vk::DescriptorType type;
		vk::ShaderStageFlags stages;
	};
	class VOIDSTAR_API DescriptorSetLayout
	{
	public:
		static DescriptorSetLayout* Create(std::vector<vk::DescriptorSetLayoutBinding>& bindings);
		inline vk::DescriptorSetLayout GetLayout() const  { return m_Layout; }
	private:
		vk::DescriptorSetLayout m_Layout;
	};
}