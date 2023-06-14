#pragma once
#include"Types.h"
#include "vulkan/vulkan.hpp"

namespace Voidstar
{
	class DescriptorPool
	{
	public:
		static SPtr<DescriptorPool> Create(vk::DescriptorType type, uint32_t descriptorCount, uint32_t maxSets);
		inline vk::DescriptorPool& GetPool() {
			return m_DescriptorPool;
		}
		std::vector<vk::DescriptorSet>AllocateDescriptorSets(uint32_t descriptorSetCount,const vk::DescriptorSetLayout* layouts);
		~DescriptorPool();
	private:
		vk::DescriptorPool m_DescriptorPool;
	};
}