#include"Prereq.h"
#include "Pipeline.h"
#include <fstream>
#include "Device.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "../Log.h"
namespace Voidstar
{
	std::vector<char> ReadFile(std::string_view filename);
	

	
	void  Pipeline::CreateComputePipeline(std::string_view pipelineName,std::string_view computeShader, const std::vector<vk::DescriptorSetLayout>& layouts)
	{
		assert(false);
		//UPtr<Pipeline> pipeline = CreateUPtr<Pipeline>();
		//
		//auto device = RenderContext::GetDevice();
		//auto computeShaderModule = CreateModule(computeShader, device->GetDevice());
		//
		//vk::PipelineShaderStageCreateInfo computeShaderStageInfo{};
		//computeShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
		//computeShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
		//computeShaderStageInfo.module = computeShaderModule;
		//computeShaderStageInfo.pName = "main";
		//
		//
		//
		//vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		//pipelineLayoutInfo.sType = vk::StructureType::ePipelineLayoutCreateInfo;
		//pipelineLayoutInfo.setLayoutCount = layouts.size();
		//pipelineLayoutInfo.pSetLayouts = layouts.data();
		//pipeline->m_PipelineLayout = device->GetDevice().createPipelineLayout(pipelineLayoutInfo, nullptr);
		//
		//
		//vk::ComputePipelineCreateInfo pipelineInfo{};
		//
		//pipelineInfo.sType = vk::StructureType::eComputePipelineCreateInfo;
		//pipelineInfo.layout = pipeline->m_PipelineLayout;
		//pipelineInfo.stage = computeShaderStageInfo;
		//pipeline->m_Pipeline = device->GetDevice().createComputePipeline(nullptr, pipelineInfo).value;
		//
		//
		//
		//vkDestroyShaderModule(device->GetDevice(), computeShaderModule, nullptr);
		//
		//
		//Renderer::Instance()->AddPipeline(pipelineName,std::move(pipeline));
	}
	Pipeline::~Pipeline()
	{
		auto& device = RenderContext::GetDevice()->GetDevice();
		device.waitIdle();
		device.destroyPipeline(m_Pipeline);
		
	
	}

	



	

}