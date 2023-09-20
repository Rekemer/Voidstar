#include"Prereq.h"
#include"DescriptorPool.h"
#include"RenderContext.h"
#include"Device.h"
#include"Log.h"

namespace Voidstar
{
	SPtr<DescriptorPool> DescriptorPool::Create(std::vector<vk::DescriptorPoolSize> poolSizes, uint32_t maxSets)
	{
		auto pool = CreateSPtr<DescriptorPool>();
		auto device = RenderContext::GetDevice();

		vk::DescriptorPoolSize poolSize{};
	
		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = maxSets;

		try
		{
			pool->m_DescriptorPool = device->GetDevice().createDescriptorPool(poolInfo);
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("failed to create descriptorPool for frames");
		}

		return pool;
	}
	std::vector<vk::DescriptorSet> DescriptorPool::AllocateDescriptorSets(uint32_t descriptorSetCount, const vk::DescriptorSetLayout* layouts)
	{
		auto device = RenderContext::GetDevice();
		vk::DescriptorSetAllocateInfo allocationInfo;
		allocationInfo.descriptorPool = m_DescriptorPool;
		allocationInfo.descriptorSetCount = descriptorSetCount;
		allocationInfo.pSetLayouts = layouts;
		


		try
		{
			auto descriptorSets = device->GetDevice().allocateDescriptorSets(allocationInfo);
			return descriptorSets;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to allocate descriptor set from pool for textures {0}", err.what());
		}

		
	}

	DescriptorPool::~DescriptorPool()
	{
		auto device = RenderContext::GetDevice();
		device->GetDevice().destroyDescriptorPool(m_DescriptorPool);
	}
}