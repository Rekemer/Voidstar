#pragma once
#include <string_view>
#include <unordered_map>
#include <filesystem>
#include <stack>
#include "ShaderType.h"
#include "Submission.h"
#include "vulkan/vulkan.hpp"

namespace Voidstar
{


    enum class ResourceType : uint8_t
    {
        UniformBuffer,
        StorageBuffer,
        SampledImage,
        StorageImage,
        Sampler,
    };


    struct BindingDesc {
        uint32_t set;
        uint32_t binding;
        ResourceType  kind;
        uint32_t count;        // array size (1 for non-array)
        uint32_t stride;       // for SSBO runtime array (arrayStride); 0 if N/A
        uint32_t elemSize;     // sizeof one struct in SSBO; 0 if N/A
        uint32_t format;
        ShaderType    stage;    // who uses it; helps build stage flags
    };

    struct PushConstRange {
        uint32_t offset;
        uint32_t size;
        ShaderType    stage;
    };

    struct StageMeta {
        std::string path;
        ShaderType stage;
        std::vector<BindingDesc> bindings;
        std::vector<PushConstRange> pushConsts;
        vk::ShaderModule module;
        uint32_t localSize[3] = { 1,1,1 }; // compute only
    };

   
    struct ProgramMeta {
        std::vector<StageMeta> stages;

        std::vector<BindingDesc> merged;   
        std::vector<PushConstRange> pushes; 


        std::vector <DescriptorLayoutKey> descriptorKey;
        PipelineLayoutKey pipelineKey;

        //uint64_t layoutKey = 0;             
    };


    struct DescriptorLayoutKey {std::vector<BindingDesc> bindings; };
    struct PipelineLayoutKey 
    {
        // multiple sets
        std::vector<DescriptorLayoutKey> descriptorsSetLayouts;
    };

    struct BufferWrite {
        uint32_t binding, arrayIndex;
        vk::DescriptorType type;        
        BufferHandle bufferHandle;       
        vk::DeviceSize offset, range;
    };
    struct DescriptorWriteKey { DescriptorLayoutKey key; std::vector<BufferWrite> buffers; };

	class ShaderCompiler
	{
	public:
		void Init();
		void Compile(std::filesystem::path shaderPath);
        void Link(ProgramHandle handle, uint8_t shaderAmount);

		std::unordered_map<ProgramHandle, ProgramMeta> m_Programs;
		std::unordered_map<PipelineLayoutKey, vk::PipelineLayout> m_PipelineLayout;
		std::unordered_map<DescriptorLayoutKey, vk::DescriptorSetLayout> m_DescriptorLayout;
		std::unordered_map<DescriptorWriteKey, vk::DescriptorSet> m_DescriptorSet;



	private:

        std::stack<StageMeta> m_StageMetas;


	};
}