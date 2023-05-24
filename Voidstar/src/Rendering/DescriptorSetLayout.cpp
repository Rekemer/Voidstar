#include"Prereq.h"
#include"DescriptorSetLayout.h"
#include"RenderContext.h"
#include"Device.h"
#include"Log.h"
namespace Voidstar
{
	DescriptorSetLayout* DescriptorSetLayout::Create(DescrriptorSetLayoutSpec& specs)
	{
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		

		auto device = RenderContext::GetDevice();

			/*
				typedef struct VkDescriptorSetLayoutBinding {
					uint32_t              binding;
					VkDescriptorType      descriptorType;
					uint32_t              descriptorCount;
					VkShaderStageFlags    stageFlags;
					const VkSampler*      pImmutableSamplers;
				} VkDescriptorSetLayoutBinding;
			*/

			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding =0;
			layoutBinding.descriptorType = specs.type;
			layoutBinding.stageFlags = specs.flags;
			layoutBinding.descriptorCount = 1;
			layoutBindings.push_back(layoutBinding);
		
		//layoutBindings[0].descriptorCount = bindings.counts[0];
		/*
			typedef struct VkDescriptorSetLayoutCreateInfo {
				VkStructureType                        sType;
				const void*                            pNext;
				VkDescriptorSetLayoutCreateFlags       flags;
				uint32_t                               bindingCount;
				const VkDescriptorSetLayoutBinding*    pBindings;
			} VkDescriptorSetLayoutCreateInfo;
		*/
		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits();
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = layoutBindings.data();
		DescriptorSetLayout* layout = new DescriptorSetLayout();
	


		try
		{
			layout->m_Layout = device->GetDevice().createDescriptorSetLayout(layoutInfo);
		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error("Failed to create Descriptor Set Layout");
			return nullptr;
		}
		return layout;
	}
}