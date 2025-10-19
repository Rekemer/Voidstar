#pragma once
#include <string_view>
#include <unordered_map>
#include <filesystem>
#include <stack>
#include <set>
#include <vector>
#include "ShaderType.h"
#include "Submission.h"
#include "../Util.h"
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
        ShaderType   access;    // who uses it; helps build stage flags
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

   
	inline vk::ShaderStageFlags mapAccess(ShaderType type) {
		vk::ShaderStageFlags out{};
		if (type == ShaderType::VERTEX)   
			out |= vk::ShaderStageFlagBits::eVertex;
		if (type == ShaderType::TESS_CONTROL)
			out |= vk::ShaderStageFlagBits::eTessellationControl;
		if (type == ShaderType::TESS_EVALUATION)
			out |= vk::ShaderStageFlagBits::eTessellationEvaluation;
		if (type == ShaderType::FRAGMENT)
			out |= vk::ShaderStageFlagBits::eFragment;
		if (type == ShaderType::COMPUTE)
			out |= vk::ShaderStageFlagBits::eCompute;
		return out;
	}
    inline vk::ShaderStageFlagBits map(ShaderType type)
    {
        switch (type)
        {
        case Voidstar::ShaderType::VERTEX:
            return vk::ShaderStageFlagBits::eVertex;
            break;
        case Voidstar::ShaderType::FRAGMENT:
            return vk::ShaderStageFlagBits::eFragment;
            break;
        case Voidstar::ShaderType::COMPUTE:
            return vk::ShaderStageFlagBits::eCompute;
            break;
        case Voidstar::ShaderType::TESS_CONTROL:
            return vk::ShaderStageFlagBits::eTessellationControl;
            break;
        case Voidstar::ShaderType::TESS_EVALUATION:
            return vk::ShaderStageFlagBits::eTessellationEvaluation;
            break;
        default:
            assert(false);
            break;
        }
    }


	struct DescriptorLayoutKey
	{
		int set;
		std::set <BindingDesc> bindings;
	};

	struct PipelineLayoutKey
	{
		// multiple sets
		std::vector<DescriptorLayoutKey> layoutKeys;
	};


	struct PipelineKey
	{
		ProgramHandle program;
		RenderState rs;
		PipelineLayoutKey layout;
		RenderPassHandle_ renderPass;
	};

	

	


	inline bool operator==(const BindingDesc& a, const BindingDesc& b) {
		return a.set == b.set && a.binding == b.binding && a.kind == b.kind &&
			a.count == b.count && a.stride == b.stride && a.elemSize == b.elemSize &&
			a.format == b.format && a.access == b.access;
	}
	inline bool operator<(const BindingDesc& a, const BindingDesc& b)
	{
		if (a.set != b.set)      return a.set < b.set;
		if (a.binding != b.binding)  return a.binding < b.binding;
		return static_cast<uint32_t>(a.access) < static_cast<uint32_t>(b.access);
	}
	inline bool operator==(const PushConstRange& a, const PushConstRange& b) {
		return a.offset == b.offset && a.size == b.size && a.stage == b.stage;
	}

	inline bool operator==(const DescriptorLayoutKey& a, const DescriptorLayoutKey& b) {
		return a.set == b.set && a.bindings == b.bindings; // relies on BindingDesc::operator==
	}

	inline bool operator==(const PipelineLayoutKey& a, const PipelineLayoutKey& b) {
		return a.layoutKeys == b.layoutKeys;
	}
	inline bool operator==(const PipelineKey& a ,const PipelineKey& b) noexcept {
		return a.program == b.program
			&& a.rs == b.rs
			&& a.renderPass == b.renderPass
			&& a.layout == b.layout;
	}

	struct BindingDescHash {
		size_t operator()(const BindingDesc& b) const noexcept {
			size_t h = 0;
			util::hash_combine(h, b.set);
			util::hash_combine(h, b.binding);
			util::hash_combine(h, static_cast<uint32_t>(b.kind));
			util::hash_combine(h, b.count);
			util::hash_combine(h, b.stride);
			util::hash_combine(h, b.elemSize);
			util::hash_combine(h, b.format);
			util::hash_combine(h, static_cast<uint32_t>(b.access));
			return h;
		}
	};

	struct DescriptorLayoutKeyHash {
		size_t operator()(const DescriptorLayoutKey& k) const noexcept {
			size_t h = 0;
			util::hash_combine(h, k.set);
			for (auto const& b : k.bindings) {
				util::hash_combine(h, BindingDescHash{}(b));
			}
			return h;
		}
	};

	struct PushConstRangeHash {
		size_t operator()(const PushConstRange& p) const noexcept {
			size_t h = 0;
			util::hash_combine(h, p.offset);
			util::hash_combine(h, p.size);
			util::hash_combine(h, static_cast<uint32_t>(p.stage));
			return h;
		}
	};

	struct PipelineLayoutKeyHash {
		size_t operator()(const PipelineLayoutKey& k) const noexcept {
			size_t h = 0;
			for (auto const& d : k.layoutKeys) {
				util::hash_combine(h, DescriptorLayoutKeyHash{}(d));
			}

			return h;
		}
	};
	struct PipelineKeyHash {
		std::size_t operator()(const PipelineKey& k) const noexcept {
			std::size_t h = 0;
			util::hash_combine(h, std::hash<ProgramHandle>{}(k.program));
			util::hash_combine(h, RenderStateHash{}(k.rs));
			util::hash_combine(h, PipelineLayoutKeyHash{}(k.layout));
			util::hash_combine(h, k.renderPass);
			return h;
		}
	};



	struct ProgramMeta {
		std::vector<StageMeta> stages;

		std::vector<BindingDesc> merged;
		std::vector<PushConstRange> pushes;


		std::vector <DescriptorLayoutKey> descriptorKey;

		//PipelineLayoutKey pipelineKey;

		//uint64_t layoutKey = 0;             
	};
    inline vk::DescriptorType map(ResourceType type)
    {
        switch (type)
        {
        case ResourceType::UniformBuffer:
            return vk::DescriptorType::eUniformBuffer;

        case ResourceType::StorageBuffer:
            return vk::DescriptorType::eStorageBuffer;

        case ResourceType::SampledImage:
            return vk::DescriptorType::eSampledImage;

        case ResourceType::StorageImage:
            return vk::DescriptorType::eStorageImage;

        case ResourceType::Sampler:
            return vk::DescriptorType::eSampler;

        default:
            assert(false && "Unknown ResourceType");
        }
    }

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