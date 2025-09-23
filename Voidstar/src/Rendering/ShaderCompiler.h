#pragma once
#include <string_view>
#include <unordered_map>
#include <filesystem>
#include <stack>
#include <vector>
#include "ShaderType.h"
#include "Submission.h"
#include "vulkan/vulkan.hpp"

namespace Voidstar
{
    namespace util {
        template <class T>
        inline void hash_combine(std::size_t& seed, const T& v) {
            std::hash<T> h;
            seed ^= h(v) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        }
    }

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
        std::unordered_map<int, std::vector<BindingDesc>> bindings;
        std::vector<PushConstRange> pushConsts;
        vk::ShaderModule module;
        uint32_t localSize[3] = { 1,1,1 }; // compute only
    };

    struct ProgramMeta {
        std::vector<StageMeta> stages;

        std::vector<BindingDesc> merged;
        std::vector<PushConstRange> pushes;


        std::vector <DescriptorLayoutKey> descriptorKey;
        
        //PipelineLayoutKey pipelineKey;

        //uint64_t layoutKey = 0;             
    };



	class ShaderCompiler
	{
	public:
		void Init();
		void Compile(const std::filesystem::path& shaderPath);
        void Link(ProgramHandle handle, uint8_t shaderAmount);

        std::unordered_map<ProgramHandle, ProgramMeta> m_Programs;


	private:

        std::stack<StageMeta> m_StageMetas;


	};
}