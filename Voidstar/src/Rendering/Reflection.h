#pragma once
#include <cstdint>
#include "ShaderType.h"
#include  <vector>
namespace Voidstar
{
    
        
    enum class ResourceType : uint8_t 
    {   UniformBuffer,
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
        ShaderType stage;
        std::vector<BindingDesc> bindings;
        std::vector<PushConstRange> pushConsts;
        uint32_t localSize[3] = { 1,1,1 }; // compute only
    };

    // “Program” = set of stages (e.g., VS+PS, or CS alone)
    struct ProgramMeta {
        std::vector<BindingDesc> merged;     // merged by (set,binding) with combined stage flags
        std::vector<PushConstRange> pushes;  // merged
        uint64_t layoutKey = 0;              // hash of merged layout
    };
}