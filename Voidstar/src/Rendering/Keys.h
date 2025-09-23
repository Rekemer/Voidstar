#pragma once
#include <vector>
#include <set>
#include "Submission.h"
#include "ShaderCompiler.h"

namespace Voidstar
{
	struct DescriptorLayoutKey
	{
		ShaderType access;
		int set;
		std::set <BindingDesc> bindings;
	};

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



	inline bool operator==(const BindingDesc& a, const BindingDesc& b) {
		return a.set == b.set && a.binding == b.binding && a.kind == b.kind &&
			a.count == b.count && a.stride == b.stride && a.elemSize == b.elemSize &&
			a.format == b.format && a.stage == b.stage;
	}
	inline bool operator<(const BindingDesc& a, const BindingDesc& b)
	{
		if (a.set != b.set)      return a.set < b.set;
		if (a.binding != b.binding)  return a.binding < b.binding;
		return static_cast<uint32_t>(a.stage) < static_cast<uint32_t>(b.stage);
	}
	inline bool operator==(const PushConstRange& a, const PushConstRange& b) {
		return a.offset == b.offset && a.size == b.size && a.stage == b.stage;
	}

	inline bool operator==(const DescriptorLayoutKey& a, const DescriptorLayoutKey& b) {
		return a.access == b.access && a.set == b.set && a.bindings == b.bindings; // relies on BindingDesc::operator==
	}


	inline bool operator==(const BufferWrite& a, const BufferWrite& b) {
		return a.binding == b.binding && a.arrayIndex == b.arrayIndex &&
			a.type == b.type && a.bufferHandle == b.bufferHandle &&
			a.offset == b.offset && a.range == b.range;
	}

	inline bool operator==(const PipelineLayoutKey& a, const PipelineLayoutKey& b) {
		return a.descriptorsSetLayouts == b.descriptorsSetLayouts;
	}


	inline bool operator==(const DescriptorWriteKey& a, const DescriptorWriteKey& b) {
		return a.key == b.key && a.buffers == b.buffers;
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
			util::hash_combine(h, static_cast<uint32_t>(b.stage));
			return h;
		}
	};

	struct DescriptorLayoutKeyHash {
		size_t operator()(const DescriptorLayoutKey& k) const noexcept {
			size_t h = 0;
			util::hash_combine(h, k.set);
			util::hash_combine(h, k.access);
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
			for (auto const& d : k.descriptorsSetLayouts) {
				util::hash_combine(h, DescriptorLayoutKeyHash{}(d));
			}

			return h;
		}
	};

	struct BufferWriteHash {
		size_t operator()(const BufferWrite& w) const noexcept {
			size_t h = 0;
			util::hash_combine(h, w.binding);
			util::hash_combine(h, w.arrayIndex);
			util::hash_combine(h, static_cast<uint32_t>(w.type));
			util::hash_combine(h, static_cast<uint64_t>(w.bufferHandle.idx));
			util::hash_combine(h, static_cast<uint64_t>(w.offset));
			util::hash_combine(h, static_cast<uint64_t>(w.range));
			return h;
		}
	};

	struct DescriptorWriteKeyHash {
		size_t operator()(const DescriptorWriteKey& k) const noexcept {
			size_t h = DescriptorLayoutKeyHash{}(k.key);
			for (auto const& bw : k.buffers) {
				util::hash_combine(h, BufferWriteHash{}(bw));
			}
			return h;
		}
	};
}