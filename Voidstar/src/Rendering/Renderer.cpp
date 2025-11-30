
#include"Prereq.h"
#include "Renderer.h"
#include <set>

#include "../Window.h"
#include "../Log.h"
#include "glfw3.h"
#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"
#include "SupportStruct.h"

#include "Buffer.h"
#include "IndexBuffer.h"
#include "RenderContext.h"
#include "DescriptorSetLayout.h"
#include "Camera.h"
#include "Image.h"
#include "DescriptorPool.h"
#include "Model.h"
#include "../Application.h"
#include <random>
#include "CommandPoolManager.h"

#include <filesystem>
#include <cstdlib>
#include <algorithm>

#include "Pipeline.h"
#include <random>
#include "Initializers.h"
#include "input.h"
#include  "Binder.h"
#include  "Generation.h"
#include"Settings.h"
#include"Sync.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <fstream>
#include <algorithm>




namespace std
{
	template<>
	struct hash<std::pair<int,int>>
	{
		size_t operator()(const std::pair<int, int>& key) const
		{
			std::string keyString = std::to_string(key.first) + std::to_string(key.second);
			return std::hash<std::string>()(keyString);
		}
	};

}



namespace Voidstar
{
	inline vk::ImageViewType pickViewType(uint32_t width, uint32_t height,
		uint32_t layers, bool cube) {
		if (cube && layers >= 6) return layers > 6 ? vk::ImageViewType::eCubeArray
			: vk::ImageViewType::eCube;
		return layers > 1 ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
	}



	vk::Format map(TextureFormat f) {
		switch (f) {
		case TextureFormat::R8_UNORM:   return vk::Format::eR8Unorm;
		case TextureFormat::R8_SNORM:   return vk::Format::eR8Snorm;
		case TextureFormat::R8_UINT:    return vk::Format::eR8Uint;
		case TextureFormat::R8_SINT:    return vk::Format::eR8Sint;
		case TextureFormat::R8_SRGB:    return vk::Format::eR8Srgb;

		case TextureFormat::RG8_UNORM:  return vk::Format::eR8G8Unorm;
		case TextureFormat::RG8_SNORM:  return vk::Format::eR8G8Snorm;
		case TextureFormat::RG8_UINT:   return vk::Format::eR8G8Uint;
		case TextureFormat::RG8_SINT:   return vk::Format::eR8G8Sint;

		case TextureFormat::RGBA8_UNORM:return vk::Format::eR8G8B8A8Unorm;
		case TextureFormat::RGBA8_SNORM:return vk::Format::eR8G8B8A8Snorm;
		case TextureFormat::RGBA8_UINT: return vk::Format::eR8G8B8A8Uint;
		case TextureFormat::RGBA8_SINT: return vk::Format::eR8G8B8A8Sint;
		case TextureFormat::RGBA8_SRGB: return vk::Format::eR8G8B8A8Srgb;

		case TextureFormat::BGRA8_UNORM:return vk::Format::eB8G8R8A8Unorm;
		case TextureFormat::BGRA8_SRGB: return vk::Format::eB8G8R8A8Srgb;

		case TextureFormat::RGB10A2_UNORM:   return vk::Format::eA2B10G10R10UnormPack32;
		case TextureFormat::R11G11B10_UFLOAT:return vk::Format::eB10G11R11UfloatPack32;

		case TextureFormat::R16_UNORM:  return vk::Format::eR16Unorm;
		case TextureFormat::R16_SNORM:  return vk::Format::eR16Snorm;
		case TextureFormat::R16_UINT:   return vk::Format::eR16Uint;
		case TextureFormat::R16_SINT:   return vk::Format::eR16Sint;
		case TextureFormat::R16_SFLOAT: return vk::Format::eR16Sfloat;

		case TextureFormat::RG16_UNORM: return vk::Format::eR16G16Unorm;
		case TextureFormat::RG16_SNORM: return vk::Format::eR16G16Snorm;
		case TextureFormat::RG16_UINT:  return vk::Format::eR16G16Uint;
		case TextureFormat::RG16_SINT:  return vk::Format::eR16G16Sint;
		case TextureFormat::RG16_SFLOAT:return vk::Format::eR16G16Sfloat;

		case TextureFormat::RGBA16_UNORM:return vk::Format::eR16G16B16A16Unorm;
		case TextureFormat::RGBA16_SNORM:return vk::Format::eR16G16B16A16Snorm;
		case TextureFormat::RGBA16_UINT: return vk::Format::eR16G16B16A16Uint;
		case TextureFormat::RGBA16_SINT: return vk::Format::eR16G16B16A16Sint;
		case TextureFormat::RGBA16_SFLOAT:return vk::Format::eR16G16B16A16Sfloat;

		case TextureFormat::R32_UINT:   return vk::Format::eR32Uint;
		case TextureFormat::R32_SINT:   return vk::Format::eR32Sint;
		case TextureFormat::R32_SFLOAT: return vk::Format::eR32Sfloat;

		case TextureFormat::RG32_UINT:  return vk::Format::eR32G32Uint;
		case TextureFormat::RG32_SINT:  return vk::Format::eR32G32Sint;
		case TextureFormat::RG32_SFLOAT:return vk::Format::eR32G32Sfloat;

		case TextureFormat::RGB32_UINT: return vk::Format::eR32G32B32Uint;
		case TextureFormat::RGB32_SINT: return vk::Format::eR32G32B32Sint;
		case TextureFormat::RGB32_SFLOAT:return vk::Format::eR32G32B32Sfloat;

		case TextureFormat::RGBA32_UINT:return vk::Format::eR32G32B32A32Uint;
		case TextureFormat::RGBA32_SINT:return vk::Format::eR32G32B32A32Sint;
		case TextureFormat::RGBA32_SFLOAT:return vk::Format::eR32G32B32A32Sfloat;

		case TextureFormat::D16_UNORM:        return vk::Format::eD16Unorm;
		case TextureFormat::X8_D24_UNORM:     return vk::Format::eX8D24UnormPack32;
		case TextureFormat::D32_SFLOAT:       return vk::Format::eD32Sfloat;
		case TextureFormat::D24_UNORM_S8_UINT:return vk::Format::eD24UnormS8Uint;
		case TextureFormat::D32_SFLOAT_S8_UINT:return vk::Format::eD32SfloatS8Uint;
		case TextureFormat::S8_UINT:          return vk::Format::eS8Uint;

		case TextureFormat::BC1_RGBA_UNORM:   return vk::Format::eBc1RgbaUnormBlock;
		case TextureFormat::BC1_RGBA_SRGB:    return vk::Format::eBc1RgbaSrgbBlock;
		case TextureFormat::BC3_RGBA_UNORM:   return vk::Format::eBc3UnormBlock;
		case TextureFormat::BC3_RGBA_SRGB:    return vk::Format::eBc3SrgbBlock;
		case TextureFormat::BC4_R_UNORM:      return vk::Format::eBc4UnormBlock;
		case TextureFormat::BC4_R_SNORM:      return vk::Format::eBc4SnormBlock;
		case TextureFormat::BC5_RG_UNORM:     return vk::Format::eBc5UnormBlock;
		case TextureFormat::BC5_RG_SNORM:     return vk::Format::eBc5SnormBlock;
		case TextureFormat::BC7_RGBA_UNORM:   return vk::Format::eBc7UnormBlock;
		case TextureFormat::BC7_RGBA_SRGB:    return vk::Format::eBc7SrgbBlock;

		default: return vk::Format::eUndefined;
		}
	}
	vk::SampleCountFlagBits map(SampleCount samples)
	{
		switch (samples)
		{
		case SampleCount::e1:  return vk::SampleCountFlagBits::e1;
		case SampleCount::e2:  return vk::SampleCountFlagBits::e2;
		case SampleCount::e4:  return vk::SampleCountFlagBits::e4;
		case SampleCount::e8:  return vk::SampleCountFlagBits::e8;
		case SampleCount::e16: return vk::SampleCountFlagBits::e16;
		case SampleCount::e32: return vk::SampleCountFlagBits::e32;
		case SampleCount::e64: return vk::SampleCountFlagBits::e64;
		default:               return vk::SampleCountFlagBits::e1;
		}
	}
	struct BufferVkMapping{
		vk::BufferUsageFlags       usage{};
		vk::MemoryPropertyFlags    mem{};
		// optional: default descriptor type for binding
		//vk::DescriptorType         dscType = vk::DescriptorType::eMaxEnum;
	};
	struct ImageVkMapping {
		vk::ImageUsageFlags        usage{};
		vk::MemoryPropertyFlags    mem{};
		vk::ImageLayout            defaultInitial = vk::ImageLayout::eUndefined;
		vk::ImageLayout            defaultFinal = vk::ImageLayout::eUndefined; // you decide based on pipeline
		vk::ImageCreateFlags       createFlags{};
	};
	inline BufferVkMapping mapBuffer(ResourceUsage u) {
		BufferVkMapping m{};

		// Usages
		if (has(u, ResourceUsage::Vertex))   m.usage |= vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		if (has(u, ResourceUsage::Index))    m.usage |= vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		if (has(u, ResourceUsage::Indirect)) m.usage |= vk::BufferUsageFlagBits::eIndirectBuffer;
		if (has(u, ResourceUsage::Uniform)) { m.usage |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst; /*m.dscType = vk::DescriptorType::eUniformBuffer; */}
		if (has(u, ResourceUsage::Sampled)) { m.usage |= vk::BufferUsageFlagBits::eUniformTexelBuffer | vk::BufferUsageFlagBits::eTransferDst; /*m.dscType = vk::DescriptorType::eUniformTexelBuffer; */}
		if (has(u, ResourceUsage::StorageRead) || has(u, ResourceUsage::StorageWrite)) {
			m.usage |= vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
			/*m.dscType = vk::DescriptorType::eStorageBuffer;*/
		}
		if (has(u, ResourceUsage::TransferSrc)) m.usage |= vk::BufferUsageFlagBits::eTransferSrc;
		if (has(u, ResourceUsage::TransferDst)) m.usage |= vk::BufferUsageFlagBits::eTransferDst;
		if (m.usage == vk::BufferUsageFlags{})  m.usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst; // sane default

		// Memory policy
		if (has(u, ResourceUsage::Upload)) {
			m.mem |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		}
		else if (has(u, ResourceUsage::Readback)) {
			m.mem |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached;
		}
		else {
			m.mem |= vk::MemoryPropertyFlagBits::eDeviceLocal;
		}

		return m;
	}

	inline ImageVkMapping mapImage(ResourceUsage u) {
		ImageVkMapping m{};
		// Usages
		if (has(u, ResourceUsage::ColorTarget))   m.usage |= vk::ImageUsageFlagBits::eColorAttachment;
		if (has(u, ResourceUsage::DepthStencil))  m.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		if (has(u, ResourceUsage::Sampled))       m.usage |= vk::ImageUsageFlagBits::eSampled;
		if (has(u, ResourceUsage::StorageWrite) || has(u, ResourceUsage::StorageRead))
			m.usage |= vk::ImageUsageFlagBits::eStorage;
		if (has(u, ResourceUsage::TransferSrc))   m.usage |= vk::ImageUsageFlagBits::eTransferSrc;
		if (has(u, ResourceUsage::TransferDst))   m.usage |= vk::ImageUsageFlagBits::eTransferDst;

		// Shape
		if (has(u, ResourceUsage::Cube))          m.createFlags |= vk::ImageCreateFlagBits::eCubeCompatible;

		// Memory
		if (has(u, ResourceUsage::Upload))        m.mem |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		else if (has(u, ResourceUsage::Readback)) m.mem |= vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached;
		else                                      m.mem |= vk::MemoryPropertyFlagBits::eDeviceLocal;

		// Default layouts (you can override per pass)
		if (has(u, ResourceUsage::ColorTarget)) {
			m.defaultInitial = vk::ImageLayout::eUndefined;                 // if you clear
			m.defaultFinal = has(u, ResourceUsage::Sampled)
				? vk::ImageLayout::eShaderReadOnlyOptimal      // write→sample pattern
				: vk::ImageLayout::eColorAttachmentOptimal;    // keep as RT
		}
		else if (has(u, ResourceUsage::DepthStencil)) {
			m.defaultInitial = vk::ImageLayout::eUndefined;
			m.defaultFinal = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		}
		else if (has(u, ResourceUsage::Sampled)) {
			m.defaultInitial = vk::ImageLayout::eUndefined;
			m.defaultFinal = vk::ImageLayout::eShaderReadOnlyOptimal;
		}

		return m;
	}

	inline vk::Filter map(FilterMode f) {
		return (f == FilterMode::Linear) ? vk::Filter::eLinear : vk::Filter::eNearest;
	}
	inline vk::MemoryPropertyFlags DeriveAttachmentMemoryPrefs(AttachmentHint hint) {
		
		vk::MemoryPropertyFlags flags;
		// Attachments are GPU-written; keep them device-local.
		flags = vk::MemoryPropertyFlagBits::eDeviceLocal;

		

		//// Transient attachments: try lazily allocated if supported.
		//if (HasFlag (hint, AttachmentHint::Transient)) {
		//	// Only valid with VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT (you already set that).
		//	// Prefer lazy, but don’t *require* it so we can gracefully fall back.
		//	p.preferred |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
		//}

		//// If we will sample / use as storage / input / read back later,
		//// we must *not* use lazy memory (can’t be lazily allocated).
		//if (Has(hint, AttachmentHint::SampledLater) ||
		//	Has(hint, AttachmentHint::Storage) ||
		//	Has(hint, AttachmentHint::InputAttachment) ||
		//	Has(hint, AttachmentHint::Readback)) {
		//	p.banned |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
		//}

		//// Large render targets (e.g., 4K MSAA) often benefit from dedicated allocs.
		//if (Has(hint, AttachmentHint::ResolveSrc) || Has(hint, AttachmentHint::ResolveDst) ||
		//	Has(hint, AttachmentHint::Storage)) {
		//	p.dedicatedPreferred = true;
		//}

		// Presentable images: you don’t allocate/bind; swapchain does it.
		// (No prefs needed.) Just skip using prefs for Presentable.
		return flags;
	}
	vk::ImageUsageFlags  map(AttachmentHint hint)
	{
		vk::ImageUsageFlags  usage{};

		//const bool isDS = IsDepthStencilFormat(format);
		//usage |= isDS ? vk::ImageUsageFlagBits::eDepthStencilAttachment
		//	: vk::ImageUsageFlagBits::eColorAttachment;

		// Transient attachments (don’t need to preserve contents)
		if (HasFlag(hint, AttachmentHint::Transient)) {
			usage |= vk::ImageUsageFlagBits::eTransientAttachment;
			//preferLazyMemory = true;   // try lazily-allocated heap if supported
		}

		// Will be sampled in a later pass
		if (HasFlag(hint, AttachmentHint::SampledLater)) {
			usage |= vk::ImageUsageFlagBits::eSampled;
			//d.preferLazyMemory = false;  // sampling requires residency
		}

		// Storage image (compute/raster UAV)
		if (HasFlag(hint, AttachmentHint::Storage)) {
			usage |= vk::ImageUsageFlagBits::eStorage;
			//d.peferLazyMemory = false;  // storage cannot be lazy
		}

		// ResolveSrc: no special usage bit needed for subpass resolve.
		// If you plan to manual-resolve via blit/copy, enable TransferSrc:
		if (HasFlag(hint, AttachmentHint::ResolveSrc)) {
			usage |= vk::ImageUsageFlagBits::eTransferSrc;
		}

		// ResolveDst: renderpass resolve (no bit), but often useful to allow TransferDst
		// for manual resolve or clears via copy.
		if (HasFlag(hint, AttachmentHint::ResolveDst)) {
			usage |= vk::ImageUsageFlagBits::eTransferDst;
		}

		// Readback: you’ll copy from this image to a staging resource → needs TransferSrc
		if (HasFlag(hint, AttachmentHint::Readback)) {
			usage |= vk::ImageUsageFlagBits::eTransferSrc;
			//d.preferLazyMemory = false;
		}

		// Subpass input attachment
		if (HasFlag(hint, AttachmentHint::InputAttachment)) {
			usage |= vk::ImageUsageFlagBits::eInputAttachment;
			//d.preferLazyMemory = false;
		}

		// Presentable: swapchain manages the image; you don't create it.
		if (HasFlag(hint, AttachmentHint::Presentable)) {
			//handledBySwapchain = true;
			// (You generally won’t use this path with vk::ImageCreateInfo.)
			// If you do reuse the same struct for bookkeeping, keep defaults and skip creation.
		}

		// If anything requires real access (sampled/storage/input/readback), disable lazy pref.
		if (HasFlag(hint, AttachmentHint::SampledLater) ||
			HasFlag(hint, AttachmentHint::Storage) ||
			HasFlag(hint, AttachmentHint::InputAttachment) ||
			HasFlag(hint, AttachmentHint::Readback)) {
			//preferLazyMemory = false;
		}

		return usage;
	}




	class  PipelineBuilder
	{
	public:
		void AddShader(vk::ShaderModule module, vk::ShaderStageFlagBits type);
		void AddBindingDescription(const vk::VertexInputBindingDescription& bindings);
		void AddBindingDescription(const std::vector<vk::VertexInputBindingDescription>& bindings);
		void AddAttributeDescription(const std::vector<vk::VertexInputAttributeDescription>& attributes);
		void AddAttributeDescription(const vk::VertexInputAttributeDescription& attribute);
		void AddDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts);
		void AddExtent(vk::Extent2D);
		void SetTopology(vk::PrimitiveTopology topology);
		void SetPolygoneMode(vk::PolygonMode polygon);
		void AddPipelineLayout(vk::PipelineLayout layout);
		void SetControlPoints(int amountPoints);
		void WriteToDepthBuffer(bool wrtite);
		void EnableStencilTest(bool test);
		void StencilTestOp(vk::CompareOp op, vk::StencilOp fail, vk::StencilOp pass, vk::StencilOp depthFailOp);
		void SetSamples(vk::SampleCountFlagBits samples);
		void SetRenderPass(vk::RenderPass renderPass);
		void SetSubpassIndex(int amount);
		void EnableBlend(bool state)
		{
			m_BlendEnable = state;
		}
		void SetStencilRefNumber(uint32_t number)
		{
			m_StencilRefNumber = number;
		}
		void SetDepthTest(bool test)
		{
			m_DepthTest = test;
		}
		void SetMasks(uint32_t compare, uint32_t write)
		{
			m_WriteMask = write;
			m_CompareMask = compare;
		}
		void SetBlendOp(vk::BlendOp op, vk::BlendFactor src, vk::BlendFactor dst)
		{
			m_BlendOp = op;
			m_BlendSrc = src;
			m_BlendDst = dst;
		}
		void SetSampleShading(vk::Bool32 state)
		{
			m_SampleShadingEnable = state; // enable sample shading in the pipeline
		}
		vk::Pipeline Build();
		~PipelineBuilder();
	private:
		std::vector<vk::ShaderModule> m_Modules;
		vk::PrimitiveTopology m_Topology = vk::PrimitiveTopology::eTriangleList;
		vk::PolygonMode m_PolygonMode;
		vk::PipelineLayout m_PipelineLayout;
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
		std::vector<vk::VertexInputBindingDescription> m_Bindings;
		std::vector<vk::VertexInputAttributeDescription> m_Attributes;
		std::vector<vk::DescriptorSetLayout> m_DescriptorSetLayouts;
		vk::Extent2D m_Extent;
		vk::SampleCountFlagBits m_Samples;
		int m_PatchControlPoints = -1;
		bool m_WriteToDepthBuffer = false;
		bool m_StencilTest = false;
		bool m_DepthTest = false;
		uint32_t m_WriteMask = 0xff;
		uint32_t m_CompareMask = 0xff;
		vk::CompareOp  m_StencilOp = vk::CompareOp::eLess;
		vk::StencilOp m_StencilFailOp = vk::StencilOp::eReplace;
		vk::StencilOp m_StencilPassOp = vk::StencilOp::eReplace;
		vk::StencilOp m_DepthFailOp = vk::StencilOp::eReplace;
		uint32_t m_StencilRefNumber = 0;
		vk::RenderPass m_RenderPass;
		int m_SubpassNumber = 0;

		vk::Bool32 m_BlendEnable = VK_TRUE;
		vk::Bool32 m_SampleShadingEnable = VK_FALSE;
		float m_MinSampleShading = .2f;
		vk::BlendOp m_BlendOp = vk::BlendOp::eAdd;
		vk::BlendFactor m_BlendSrc = vk::BlendFactor::eSrcAlpha;
		vk::BlendFactor m_BlendDst = vk::BlendFactor::eOneMinusSrcAlpha;

	};


	void PipelineBuilder::AddShader(vk::ShaderModule module, vk::ShaderStageFlagBits type)
	{
		m_Modules.push_back(module);
		{
			vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
			vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
			vertexShaderInfo.stage = type;
			vertexShaderInfo.module = module;
			vertexShaderInfo.pName = "main";
			m_ShaderStages.push_back(vertexShaderInfo);
		}
	}


	void PipelineBuilder::AddBindingDescription(const vk::VertexInputBindingDescription& bindings)
	{
		m_Bindings.push_back(bindings);
	}
	void PipelineBuilder::AddBindingDescription(const std::vector<vk::VertexInputBindingDescription>& bindings)
	{
		m_Bindings.insert(m_Bindings.end(), bindings.begin(), bindings.end());
	}
	void PipelineBuilder::AddAttributeDescription(const std::vector<vk::VertexInputAttributeDescription>& attributes)
	{
		m_Attributes = attributes;
	}
	void PipelineBuilder::AddAttributeDescription(const vk::VertexInputAttributeDescription& attribute)
	{
		m_Attributes.push_back(attribute);
	}
	void PipelineBuilder::AddDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts)
	{
		m_DescriptorSetLayouts = layouts;
	}

	void PipelineBuilder::AddExtent(vk::Extent2D size)
	{
		m_Extent = size;
	}

	void PipelineBuilder::SetTopology(vk::PrimitiveTopology topology)
	{
		m_Topology = topology;
	}

	void PipelineBuilder::SetPolygoneMode(vk::PolygonMode polygon)
	{
		m_PolygonMode = polygon;
	}

	void PipelineBuilder::AddPipelineLayout(vk::PipelineLayout layout)
	{
		m_PipelineLayout = layout;
	}

	void PipelineBuilder::SetControlPoints(int amountPoints)
	{
		assert(amountPoints > 0);
		m_PatchControlPoints = amountPoints;
	}

	void PipelineBuilder::WriteToDepthBuffer(bool wrtite)
	{
		m_WriteToDepthBuffer = write;
	}

	void PipelineBuilder::EnableStencilTest(bool test)
	{
		m_StencilTest = test;
	}

	void PipelineBuilder::StencilTestOp(vk::CompareOp op, vk::StencilOp fail, vk::StencilOp pass, vk::StencilOp depthFailOp)
	{
		m_StencilOp = op;
		m_StencilFailOp = fail;
		m_StencilPassOp = pass;
		m_DepthFailOp = depthFailOp;
	}
	
	PipelineBuilder::~PipelineBuilder()
	{
	
	}
	void PipelineBuilder::SetSamples(vk::SampleCountFlagBits samples)
	{
		m_Samples = samples;
	}

	void PipelineBuilder::SetRenderPass(vk::RenderPass renderPass)
	{
		m_RenderPass = renderPass;
	}

	void PipelineBuilder::SetSubpassIndex(int numberOfSubpass)
	{
		m_SubpassNumber = numberOfSubpass;
	}

	vk::Pipeline PipelineBuilder::Build()
	{


		auto pipeline = CreateUPtr<Pipeline>();
		/*
		* Build and return a graphics pipeline based on the given info.
		*/

		//The info for the graphics pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.flags = vk::PipelineCreateFlags();


		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexBindingDescriptionCount = m_Bindings.size();
		vertexInputInfo.pVertexBindingDescriptions = m_Bindings.data();

		vertexInputInfo.vertexAttributeDescriptionCount = m_Attributes.size();
		vertexInputInfo.pVertexAttributeDescriptions = m_Attributes.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = m_Topology;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		vk::PipelineTessellationStateCreateInfo tesselationState;
		if (m_PatchControlPoints != -1)
		{
			tesselationState.patchControlPoints = m_PatchControlPoints;

			pipelineInfo.pTessellationState = &tesselationState;
		}









		//Viewport and Scissor
		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = m_Extent.width;
		viewport.height = m_Extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vk::Rect2D scissor = {};
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		scissor.extent = m_Extent;
		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.flags = vk::PipelineViewportStateCreateFlags();
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		pipelineInfo.pViewportState = &viewportState;

		const vk::DynamicState dynamicStates[] = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
		};
		vk::PipelineDynamicStateCreateInfo createInfo{};
		createInfo.pNext = nullptr;
		createInfo.flags = {};
		createInfo.dynamicStateCount = 2;
		createInfo.pDynamicStates = &dynamicStates[0];

		pipelineInfo.pDynamicState = &createInfo;


		//Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizer.depthClampEnable = VK_FALSE; //discard out of bounds fragments, don't clamp them
		rasterizer.rasterizerDiscardEnable = VK_FALSE; //This flag would disable fragment output
		rasterizer.polygonMode = m_PolygonMode;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE; //Depth bias can be useful in shadow maps.
		pipelineInfo.pRasterizationState = &rasterizer;






		pipelineInfo.stageCount = m_ShaderStages.size();
		pipelineInfo.pStages = m_ShaderStages.data();




		vk::PipelineDepthStencilStateCreateInfo depthState;
		depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
		depthState.depthTestEnable = m_DepthTest;
		depthState.depthWriteEnable = m_WriteToDepthBuffer;
		depthState.depthCompareOp = vk::CompareOp::eLess;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = m_StencilTest;
		depthState.back.compareOp = m_StencilOp;
		depthState.back.failOp = m_StencilFailOp;
		depthState.back.passOp = m_StencilPassOp;
		depthState.back.compareMask = m_CompareMask;
		depthState.back.writeMask = m_WriteMask;
		depthState.back.reference = m_StencilRefNumber;
		depthState.front = depthState.back;

		pipelineInfo.pDepthStencilState = &depthState;

		//Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = m_SampleShadingEnable;
		multisampling.minSampleShading = m_MinSampleShading;
		multisampling.rasterizationSamples = m_Samples;
		pipelineInfo.pMultisampleState = &multisampling;

		//Color Blend
		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;


		colorBlendAttachment.blendEnable = m_BlendEnable;
		colorBlendAttachment.colorBlendOp = m_BlendOp;
		colorBlendAttachment.srcColorBlendFactor = m_BlendSrc;
		colorBlendAttachment.dstColorBlendFactor = m_BlendDst;



		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		pipelineInfo.pColorBlendState = &colorBlending;

		pipelineInfo.layout = m_PipelineLayout;




		//Renderpass

		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = m_SubpassNumber;


		//Extra stuff
		pipelineInfo.basePipelineHandle = nullptr;

		//Make the Pipeline

		vk::Pipeline graphicsPipeline;
		try
		{
			auto pipeline = RenderContext::GetDevice()->GetDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;
			Log::GetLog()->info("Pipeline is Created!");
			return pipeline;
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create Pipeline");
			return {};
		}

	}


}


namespace Voidstar
{


	static vk::BlendFactor map(BlendFactor f) {
		switch (f) {
		case BlendFactor::Zero:               return vk::BlendFactor::eZero;
		case BlendFactor::One:                return vk::BlendFactor::eOne;
		case BlendFactor::SrcColor:           return vk::BlendFactor::eSrcColor;
		case BlendFactor::OneMinusSrcColor:   return vk::BlendFactor::eOneMinusSrcColor;
		case BlendFactor::DstColor:           return vk::BlendFactor::eDstColor;
		case BlendFactor::OneMinusDstColor:   return vk::BlendFactor::eOneMinusDstColor;
		case BlendFactor::SrcAlpha:           return vk::BlendFactor::eSrcAlpha;
		case BlendFactor::OneMinusSrcAlpha:   return vk::BlendFactor::eOneMinusSrcAlpha;
		case BlendFactor::DstAlpha:           return vk::BlendFactor::eDstAlpha;
		case BlendFactor::OneMinusDstAlpha:   return vk::BlendFactor::eOneMinusDstAlpha;
		case BlendFactor::ConstColor:         return vk::BlendFactor::eConstantColor;
		case BlendFactor::OneMinusConstColor: return vk::BlendFactor::eOneMinusConstantColor;
		case BlendFactor::ConstAlpha:         return vk::BlendFactor::eConstantAlpha;
		case BlendFactor::OneMinusConstAlpha: return vk::BlendFactor::eOneMinusConstantAlpha;
		case BlendFactor::SrcAlphaSaturate:   return vk::BlendFactor::eSrcAlphaSaturate;
		}
		return vk::BlendFactor::eOne;
	}

	static vk::BlendOp map(BlendOp op) {
		switch (op) {
		case BlendOp::Add:             return vk::BlendOp::eAdd;
		case BlendOp::Subtract:        return vk::BlendOp::eSubtract;
		case BlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
		case BlendOp::Min:             return vk::BlendOp::eMin;
		case BlendOp::Max:             return vk::BlendOp::eMax;
		}
		return vk::BlendOp::eAdd;
	}

	static vk::CompareOp map(CompareOp c) {
		switch (c) {
		case CompareOp::Never:    return vk::CompareOp::eNever;
		case CompareOp::Less:     return vk::CompareOp::eLess;
		case CompareOp::Equal:    return vk::CompareOp::eEqual;
		case CompareOp::LessEqual:   return vk::CompareOp::eLessOrEqual;
		case CompareOp::Greater:  return vk::CompareOp::eGreater;
		case CompareOp::NotEqual: return vk::CompareOp::eNotEqual;
		case CompareOp::GreaterEqual:   return vk::CompareOp::eGreaterOrEqual;
		case CompareOp::Always:   return vk::CompareOp::eAlways;
		}
		return vk::CompareOp::eLessOrEqual;
	}

	static vk::CullModeFlags map(Culling c) {
		switch (c) {
		case Culling::None:  return vk::CullModeFlagBits::eNone;
		case Culling::Back:  return vk::CullModeFlagBits::eBack;
		case Culling::Front: return vk::CullModeFlagBits::eFront;
		}
		return vk::CullModeFlagBits::eBack;
	}

	static vk::PolygonMode map(Polygon p) {
		switch (p) {
		case Polygon::Fill:  return vk::PolygonMode::eFill;
		case Polygon::Line:  return vk::PolygonMode::eLine;
		case Polygon::Point: return vk::PolygonMode::ePoint;
		}
		return vk::PolygonMode::eFill;
	}

	static vk::PrimitiveTopology map(Topology t) {
		switch (t) {
		case Topology::TriList:   return vk::PrimitiveTopology::eTriangleList;
		case Topology::TriStrip:  return vk::PrimitiveTopology::eTriangleStrip;
		case Topology::LineList:  return vk::PrimitiveTopology::eLineList;
		case Topology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
		case Topology::Point:     return vk::PrimitiveTopology::ePointList;
		}
		return vk::PrimitiveTopology::eTriangleList;
	}

	static vk::StencilOp map(StencilOp o) {
		switch (o) {
		case StencilOp::Keep:      return vk::StencilOp::eKeep;
		case StencilOp::Zero:      return vk::StencilOp::eZero;
		case StencilOp::Replace:   return vk::StencilOp::eReplace;
		case StencilOp::IncrClamp: return vk::StencilOp::eIncrementAndClamp;
		case StencilOp::DecrClamp: return vk::StencilOp::eDecrementAndClamp;
		case StencilOp::Invert:    return vk::StencilOp::eInvert;
		case StencilOp::IncrWrap:  return vk::StencilOp::eIncrementAndWrap;
		case StencilOp::DecrWrap:  return vk::StencilOp::eDecrementAndWrap;
		}
		return vk::StencilOp::eKeep;
	}

	static vk::Format map(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::FLOAT:  return vk::Format::eR32Sfloat;
		case ShaderDataType::FLOAT2: return vk::Format::eR32G32Sfloat;
		case ShaderDataType::FLOAT3: return vk::Format::eR32G32B32Sfloat;
		case ShaderDataType::FLOAT4: return vk::Format::eR32G32B32A32Sfloat;
		default:
			throw std::runtime_error("Unknown ShaderDataType");
		}
	}




	std::vector<Vertex_> sphere;
	std::vector<IndexType> sphereIndicies;
	
	const int QUAD_AMOUNT = 700;

	
	std::vector<vk::DescriptorSet> Renderer::AllocateSets(size_t amount, const DescriptorLayoutKey& key)
	{
		if (m_DescriptorSet.find(key) == m_DescriptorSet.end())
		{
			auto& layout = m_DescriptorLayout.at(key);
			std::vector<vk::DescriptorSetLayout> layouts{amount, layout};
			m_DescriptorSet[key] = m_UniversalPool->AllocateDescriptorSets(amount, layouts.data());
		}
		return  m_DescriptorSet[key];
	}
	void Renderer::CleanUpLayouts()
	{
		auto device = RenderContext::GetDevice();
		for (auto [key, value] : m_Layout)
		{
			device->GetDevice().destroyDescriptorSetLayout(value->GetLayout());
		}
	}


	CommandBuffer& Renderer::GetRenderCommandBuffer(size_t frameindex)
	{
		assert(frameindex < m_RenderCommandBuffer.size());
		return m_RenderCommandBuffer[frameindex];
	}

	CommandBuffer& Renderer::GetComputeCommandBuffer(size_t frameindex)
	{
		assert(frameindex < m_ComputeCommandBuffer.size());
		return m_ComputeCommandBuffer[frameindex];
	}

	CommandBuffer& Renderer::GetTransferCommandBuffer(size_t frameindex)
	{
		assert(frameindex < m_TransferCommandBuffer.size());
		return m_TransferCommandBuffer[frameindex];
	}

	void Renderer::BeginBatch()
	{
		m_QuadIndex = 0;
		m_BatchQuad = m_BatchQuadStart;
		//m_BatchInstance = m_BatchInstanceStart;
	}

	void Renderer::DrawBatch(vk::CommandBuffer& commandBuffer,size_t offset, int index)
	{
		vk::DeviceSize offsets[] = { offset };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(m_QuadIndex, 1, 0, 0, 0);
	}
	void Renderer::DrawBatchCustom(vk::CommandBuffer& commandBuffer, size_t indexAmount,size_t offset, int index)
	{
		vk::DeviceSize offsets[] = { offset };

		{
			vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
			commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		}
		commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.drawIndexed(indexAmount, 1, m_QuadIndex-index, 0, 0);
	}

	void UpdateVertex(Vertex_*& vertex, glm::vec3 position, glm::vec2 uv,glm::vec4& color, glm::mat4& world,  int vertIndex)
	{
		vertex->Position = world * glm::vec4{ position,1 };
		vertex->UV = uv;
		vertex->Color = color;
	};
	void UpdateVerticies(Vertex_*& vertex, std::vector<Vertex_>& verticies)
	{
		assert(false);
		//UpdateVertex(vertex,verticies[0].Position, verticies[0].Color, glm::identity<glm::mat4>());
		//vertex++;																				   
		//UpdateVertex(vertex,verticies[2].Position, verticies[2].Color, glm::identity<glm::mat4>());
		//vertex++;																				   
		//UpdateVertex(vertex,verticies[3].Position, verticies[3].Color, glm::identity<glm::mat4>());
		//vertex++;																				   
		//UpdateVertex(vertex,verticies[1].Position, verticies[1].Color, glm::identity<glm::mat4>());
		//vertex++;

	};
	void Renderer::DrawTxt(vk::CommandBuffer commandBuffer, std::string_view str, glm::vec2 pos, std::map< unsigned char, Character>& characters)
	{
		float scale = 1;
		float scaleX = 1;
		auto offset = pos;
		for (auto e : str)
		{

			if (e == '\n')
			{
				offset.x = pos.x;
				pos.y -= 3* CharacterLineSpacing /64.f;
				continue;
			}
			else if (e == ' ')
			{
				offset.x += 5;
				continue;

			}
			else if (e == '\t')
			{
				offset.x += 30;
				continue;

			}
			if (characters.find(e) == characters.end()) continue;
		auto& characterData = characters.at(e);
		offset.x = offset.x + characterData.Bearing.x* scale;
		// to account for letter like p and q
		offset.y = pos.y - ( characterData.Size.y - characterData.Bearing.y)* scale;
		glm::vec4 color{ 1 };
		glm::mat4 world{ 1 };
		// left bottom
		m_BatchQuad->Position = glm::vec3{ offset.x ,offset.y,0};
		m_BatchQuad->UV = { characterData.minUv.x,characterData.maxUv.y };
		
		m_BatchQuad++;
		// right bottom
		m_BatchQuad->Position = glm::vec3{ offset.x + characterData.Size.x * scaleX ,offset.y,0 };
		m_BatchQuad->UV = { characterData.maxUv.x,characterData.maxUv.y };
		m_BatchQuad++;
		// right top
		m_BatchQuad->Position = glm::vec3{ offset.x + characterData.Size.x * scaleX,offset.y + characterData.Size.y * scale,0 };
		m_BatchQuad->UV = { characterData.maxUv.x,characterData.minUv.y };
		m_BatchQuad++;
		
		
		// left top
		m_BatchQuad->Position = glm::vec3{ offset.x ,offset.y + characterData.Size.y * scale,0 };
		m_BatchQuad->UV = { characterData.minUv.x,characterData.minUv.y };
		m_BatchQuad++;
		
		
		offset.x += characterData.Advance / 64.f* scaleX;
		m_QuadIndex += 6;
		}
	}
	void Renderer::DrawQuadScreen(vk::CommandBuffer commandBuffer)
	{
		//vk::DeviceSize offsets[] = { 0 };
		//{
		//	vk::Buffer vertexBuffers[] = { m_QuadBufferBatch->GetBuffer() };
		//	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		//
		//}
		//commandBuffer.bindIndexBuffer(m_QuadBufferBatchIndex->GetBuffer(), 0, m_QuadBufferBatchIndex->GetIndexType());
		commandBuffer.draw(6, 1, 0, 0);
	}

	void Renderer::Draw(Sphere& sphere)
	{
		DrawSphere(sphere.Pos,sphere.Scale,sphere.Color,sphere.Rot);
	}
	
	void Renderer::Draw(Quad& quad, glm::mat4& world)
	{
		DrawQuad(world, glm::vec4{1,0,1,1});
	}

	void Renderer::Draw(QuadRangle& quadrangle)
	{
		DrawQuad(quadrangle.Verticies);
	}
	void Renderer::DrawSphere(glm::vec3 pos, glm::vec3 scale, glm::vec4 color, glm::vec3 rot)
	{
		assert(false);
		//m_BatchInstance->Color = color;
		auto iden = glm::identity<glm::mat4>();
		iden = glm::translate(iden, pos);
		auto rotMatrix =  glm::rotate(iden, glm::radians(rot.x), glm::vec3{ 1,0,0 });
		rotMatrix = glm::rotate(rotMatrix, glm::radians(rot.y), glm::vec3{ 0,1,0 });
		rotMatrix = glm::scale(rotMatrix,scale);
		auto transpose = glm::transpose(rotMatrix);
		//m_BatchInstance->WorldMatrix= transpose;
		//m_BatchInstance++;
	}

	void Renderer::DrawSphereInstance(vk::CommandBuffer& commandBuffer)
	{
		assert(false);
		//vk::DeviceSize offsets[] = { 0 };
		//
		//{
		//	vk::Buffer vertexBuffers[] = { m_SphereBuffer->GetBuffer() };
		//	vk::Buffer instanceBuffers[] = { m_InstanceBuffer->GetBuffer() };
		//	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		//	commandBuffer.bindVertexBuffers(1, 1, instanceBuffers, offsets);
		//
		//}
		//commandBuffer.bindIndexBuffer(m_SphereIndexBuffer->GetBuffer(), 0, m_SphereIndexBuffer->GetIndexType());
		////auto instanceAmount = static_cast<uint64_t>(m_BatchInstance - m_BatchInstanceStart);
		//auto instanceAmount = static_cast<uint64_t>(0);
		//commandBuffer.drawIndexed(m_SphereIndexBuffer->GetIndexAmount(), instanceAmount, 0, 0, 0);
	}
	void Renderer::DrawQuad(glm::mat4& world, glm::vec4 color)
	{
		assert(false);
		//auto& verticies = quad.verticies;
		//// left bottom
		//UpdateVertex(m_BatchQuad,verticies[0].Position,color,world,0);
		//m_BatchQuad++;
		//// right bottom
		//UpdateVertex(m_BatchQuad,verticies[2].Position,color,world,2);
		//m_BatchQuad++;
		//// right top
		//UpdateVertex(m_BatchQuad, verticies[3].Position, color, world);
		//m_BatchQuad++;


		//// left top
		//UpdateVertex(m_BatchQuad, verticies[1].Position, color, world);
		//m_BatchQuad++;

		//m_QuadIndex += 6;
	}

	void Renderer::DrawQuad(std::vector<Vertex_>& verticies)
	{
		assert(false);
		//UpdateVerticies(m_BatchQuad, verticies);
		//m_QuadIndex += 6;
	}
	


	std::vector<vk::Framebuffer> _CreateFramebuffer(
		vk::RenderPass renderPass,
		int width, int height,
		RenderPassBuilder& builder,
		const std::vector<AttachmentHandle> handles)
	{

		std::vector<vk::Framebuffer> framebuffers;
		for (int i = 0; i < RenderContext::GetFrameAmount(); i++)
		{

			int colorOutputOverall = 0;
			std::vector<vk::ImageView> views;
			for (auto  type: builder.m_OutputTypes)
			{
			

				if (type == OutputType::COLOR)
				{
					views.push_back(builder.m_Color[colorOutputOverall][i]->GetImageView());
					colorOutputOverall++;
				}
				else if (type == OutputType::DEPTH)
				{
					views.push_back(builder.m_DepthStencil[0]->GetImageView());
				}
				else if (type == OutputType::RESOLVE)
				{
					views.push_back(builder.m_Resolve[i]->GetImageView());

				}
			}
			auto framebuffer = CreateVkFramebuffer(views, renderPass, width, height);
			framebuffers.push_back(framebuffer);
		}
		return framebuffers;
	}
	
	void Renderer::Init(size_t screenWidth, size_t screenHeight, std::shared_ptr<Window> window, Application* app) 
		
	{
		InitFilePath();
		m_App = app;
		m_Window=window; 
		m_ViewportWidth = screenWidth;  
		m_ViewportHeight = screenHeight;
		m_CommandPoolManager = CreateUPtr<CommandPoolManager>();
		m_Compiler.Init();
		// create instance
		CreateInstance();


		RenderContext::CreateSurface(window.get());
		RenderContext::CreateDevice();

		m_Device = RenderContext::GetDevice();

		RenderContext::CreateSwapchain(vk::Format::eB8G8R8A8Unorm,
			m_ViewportWidth, m_ViewportHeight,
			vk::PresentModeKHR::eFifo, vk::ColorSpaceKHR::eSrgbNonlinear) ;
		
		CreateSyncObjects();


		std::vector<vk::DescriptorPoolSize> pool_sizes =
		{
			{ vk::DescriptorType::eCombinedImageSampler, 64 },
			{ vk::DescriptorType::eStorageImage, 64 },
			{ vk::DescriptorType::eStorageBuffer, 64 },
			{ vk::DescriptorType::eInputAttachment, 64 },
			{ vk::DescriptorType::eUniformBuffer, 64 },
		};

		m_UniversalPool = DescriptorPool::Create(pool_sizes, 64);

	
		auto frameAmount = RenderContext::GetFrameAmount();
		m_FrameCommandPool = Renderer::Instance()->GetCommandPoolManager()->GetFreePool();
		m_RenderCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
		m_TransferCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
		m_ComputeCommandBuffer = CommandBuffer::CreateBuffers(m_FrameCommandPool, vk::CommandBufferLevel::ePrimary, frameAmount);
		
		


		// get frame amount
		auto framesAmount = RenderContext::GetFrameAmount();
		auto m_Device = RenderContext::GetDevice();
		auto bufferSize = sizeof(UniformBufferObject);
		m_UniformBuffers.resize(framesAmount);
		m_UniformBuffersMapped.resize(framesAmount);


		BufferInputChunk inputBuffer;
		inputBuffer.size = bufferSize;
		inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer;

		for (size_t i = 0; i < framesAmount; i++)
		{
			m_UniformBuffers[i] = CreateUPtr<Buffer>(inputBuffer);
			m_UniformBuffersMapped[i] = m_Device->GetDevice().mapMemory(m_UniformBuffers[i]->GetMemory(), 0, bufferSize);
		}



		SystemDescriptorLayoutKey.set = 0;
		BindingDesc desc;
		desc.binding = 0;
		desc.set = 0;
		desc.access = ShaderType::ALL;
		desc.kind = ResourceType::UniformBuffer;
		desc.count = 1;
		desc.elemSize = sizeof(UniformBufferObject);
		desc.stride = 0;
		SystemDescriptorLayoutKey.bindings.insert(desc);
		CreateDescriptorLayout(SystemDescriptorLayoutKey);


		auto sets = AllocateSets(frameAmount, SystemDescriptorLayoutKey);

		for (auto i = 0; i < frameAmount; i++)
		{
			for (auto binding : SystemDescriptorLayoutKey.bindings)
			{
				m_Device->UpdateDescriptorSet(sets[i], binding.binding, binding.count, *m_UniformBuffers[i], binding.kind);
			}
		}


		m_DefaultColorAttachment = GetAttachmentHandle();
		m_DefaultMSAAAttachment = GetAttachmentHandle();
		m_DefaultDepthAttachment = GetAttachmentHandle();

		{
			auto images = RenderContext::GetFrames();
			std::vector<TextureHandle> handles{ {GetTextureHandle(),GetTextureHandle() ,GetTextureHandle() } };
			for (int i = 0; i < handles.size(); i++)
			{
				auto handle = handles[i];
				m_Textures[handle] = images[i];
			}

			m_AttachmentManager.Init(handles, m_DefaultColorAttachment);
		}


		auto samples = RenderContext::GetDevice()->GetSamples();
		m_AttachmentManager.CreateColor(m_DefaultMSAAAttachment, vk::Format::eB8G8R8A8Unorm,
			screenWidth, screenHeight,
			samples, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
			frameAmount);

		m_AttachmentManager.CreateDepthStencil(m_DefaultDepthAttachment,
			screenWidth, screenHeight, 
			vk::SampleCountFlagBits::e1, vk::ImageUsageFlagBits::eDepthStencilAttachment,
			frameAmount);


		DEFAULT_FRAME_BUFFER = GetFrameBufferHandle();
		

		RenderPassBuilder builder;

		builder.ColorOutput(m_DefaultColorAttachment, m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
		builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
		builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
		builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetInitialLayout(vk::ImageLayout::eUndefined);
		builder.SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		builder.BuildAttachmentDesc();

		builder.DepthStencilOutput(m_DefaultDepthAttachment, m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
		builder.SetSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
		builder.SetInitialLayout(vk::ImageLayout::eUndefined);
		builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		builder.BuildAttachmentDesc();

		//builder.ResolveOutput(m_DefaultColorAttachment, m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
		//builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
		//builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
		//builder.SetStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		//builder.SetStencilSaveOp(vk::AttachmentStoreOp::eDontCare);
		//builder.SetInitialLayout(vk::ImageLayout::eUndefined);
		////builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
		//builder.SetFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		//builder.BuildAttachmentDesc();

		vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);



		//builder.AddSubpass({ 0 }, { 1 }, { 2 });
		builder.AddSubpass({ 0 }, { 1 }, {});

		builder.AddSubpassDependency(dependency0);

		

		vk::Extent2D extent = { static_cast<uint32_t>(screenWidth),static_cast<uint32_t>(screenHeight)};

		vk::ClearValue clearColor = { std::array<float, 4>{137.f / 255.f, 189.f / 255.f, 199.f / 255.f, 1.0f} };
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue{ 1.0f, 0 };
		std::vector<vk::ClearValue> clearValues{ clearColor ,clearDepth, clearColor };
		m_RenderPasses[DEFAULT_FRAME_BUFFER] = builder.Build(m_AttachmentManager, RenderContext::GetFrameAmount(), extent, clearValues);


		//AttachmentInfo_ info{ type,format,width,height,samples,hints };
		//m_AttachmentInfo[m_DefaultColorAttachment] = AttachmentInfo_{AttachmentType::COLOR,}
		m_Framebuffers[DEFAULT_FRAME_BUFFER] = _CreateFramebuffer(m_RenderPasses[DEFAULT_FRAME_BUFFER].m_RenderPass,
			extent.width, extent.height, builder,
			{ m_DefaultColorAttachment,m_DefaultDepthAttachment  });


		


#if 0
		quad = GeneratePlane(1);
		sphere = GenerateSphere(1,10, sphereIndicies);

		auto& verticies = quad.verticies;
		auto& indices = quad.indicies;
		auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
		{
			{
				SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


				{
					BufferInputChunk inputBuffer;
					inputBuffer.size = indexSize;
					inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
					inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
					m_QuadIndexBuffer = CreateUPtr<IndexBuffer>(inputBuffer, indices.size(), vk::IndexType::eUint32);

				}

				m_TransferCommandBuffer[0].BeginTransfering();
				m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_QuadIndexBuffer.get(), (void*)indices.data(), indexSize);
				m_TransferCommandBuffer[0].EndTransfering();
				m_TransferCommandBuffer[0].SubmitSingle();
			}
			{


				auto vertexSize = SizeOfBuffer(verticies.size(), verticies[0]);
				{
					BufferInputChunk inputBuffer;
					inputBuffer.size = vertexSize;
					inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
					inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

					m_QuadBuffer = CreateUPtr<Buffer>(inputBuffer);


					
					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = sizeof(Vertex) * 4 * QUAD_AMOUNT;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
						inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer;
						m_QuadBufferBatch = CreateUPtr<Buffer>(inputBuffer);
						m_BatchQuadStart = reinterpret_cast<Vertex*>( m_Device->GetDevice().mapMemory(m_QuadBufferBatch->GetMemory(),0, inputBuffer.size));

						indices.resize(QUAD_AMOUNT*3);
						int offset = 0;
						for (int i = 0; i < QUAD_AMOUNT*3; i+=6)
						{
							indices[i] = offset;
							indices[i + 1] = offset + 1;
							indices[i + 2] = offset + 2;

							indices[i + 3] = offset + 2;
							indices[i + 4] = offset + 3;
							indices[i + 5] = offset;

							offset += 4;
						}
						{
							BufferInputChunk inputBuffer;
							inputBuffer.size = SizeOfBuffer(indices.size(),indices[0]);
							inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
							inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
							m_QuadBufferBatchIndex = CreateUPtr<IndexBuffer>(inputBuffer, indices.size(), vk::IndexType::eUint32);


							auto indexSize = SizeOfBuffer(indices.size(), indices[0]);
							SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);

							m_TransferCommandBuffer[0].BeginTransfering();
							m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_QuadBufferBatchIndex.get(), (void*)indices.data(), indexSize);
							m_TransferCommandBuffer[0].EndTransfering();
							m_TransferCommandBuffer[0].SubmitSingle();

						}
					}
				}
				void* vertexData = const_cast<void*>(static_cast<const void*>(verticies.data()));
				SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
				m_TransferCommandBuffer[0].BeginTransfering();
				m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_QuadBuffer.get(), (void*)verticies.data(), vertexSize);
				m_TransferCommandBuffer[0].EndTransfering();
				m_TransferCommandBuffer[0].SubmitSingle();
			}



		}


		{
			auto indexSize = SizeOfBuffer(sphereIndicies.size(), sphereIndicies[0]);
			{
				{
					SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(indexSize);


					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = indexSize;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
						inputBuffer.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
						m_SphereIndexBuffer = CreateUPtr<IndexBuffer>(inputBuffer, sphereIndicies.size(), vk::IndexType::eUint32);

					}

					m_TransferCommandBuffer[0].BeginTransfering();
					m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_SphereIndexBuffer.get(), (void*)sphereIndicies.data(), indexSize);
					m_TransferCommandBuffer[0].EndTransfering();
					m_TransferCommandBuffer[0].SubmitSingle();
				}
				{
					auto vertexSize = SizeOfBuffer(sphere.size(), sphere[0]);
					{
						BufferInputChunk inputBuffer;
						inputBuffer.size = vertexSize;
						inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
						inputBuffer.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

						m_SphereBuffer = CreateUPtr<Buffer>(inputBuffer);
					}
					SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(vertexSize);
					m_TransferCommandBuffer[0].BeginTransfering();
					m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), m_SphereBuffer.get(), (void*)sphere.data(), vertexSize);
					m_TransferCommandBuffer[0].EndTransfering();
					m_TransferCommandBuffer[0].SubmitSingle();
				}
			}
		}
		

		{
			BufferInputChunk inputBuffer;
			inputBuffer.size = sizeof(InstanceData) * 30;
			inputBuffer.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			inputBuffer.usage = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer;
			m_InstanceBuffer = CreateUPtr<Buffer>(inputBuffer);
			m_BatchInstanceStart = reinterpret_cast<InstanceData*>(m_Device->GetDevice().mapMemory(m_InstanceBuffer->GetMemory(), 0, inputBuffer.size));

		}

		
#endif // 0
			
	}

	
	

	

	



	


	
	void Renderer::Draw(Drawable& drawable)
	{
		drawable.m_Self->Draw();
	}

	void Renderer::CreateSyncObjects()
	{	
		auto frameAmount = RenderContext::GetFrameAmount();
		m_ComputeInFlightFences.resize(frameAmount);
		m_ComputeFinishedSemaphores = Semaphore::CreateBinarySemaphore(RenderContext::GetFrameAmount());
		m_ImageAvailableSemaphore = Semaphore::CreateBinarySemaphore(RenderContext::GetFrameAmount());
		m_RenderFinishedSemaphore = Semaphore::CreateBinarySemaphore(RenderContext::GetFrameAmount());
		m_TimelineSemaphore = Semaphore::CreateTimelineSemaphore(RenderContext::GetFrameAmount(),0);
		m_InFlightFence.resize(frameAmount);
	
	}




	void Renderer::RecreateSwapchain()
	{
		m_ViewportWidth = 0;
		m_ViewportHeight = 0;
		while (m_ViewportWidth == 0 || m_ViewportHeight == 0) {
			glfwGetFramebufferSize(m_Window->GetRaw(), &m_ViewportWidth, &m_ViewportHeight);
			glfwWaitEvents();
		}


		m_Device->GetDevice().waitIdle();


		SwapChainSupportDetails support;
		auto device = RenderContext::GetDevice();
		auto surface = RenderContext::GetSurface();
		support.AvailableCapabilities = device->GetDevicePhys().getSurfaceCapabilitiesKHR(*surface);
		support.AvailablePresentModes = device->GetDevicePhys().getSurfacePresentModesKHR(*surface);
		support.AvailableFormats = device->GetDevicePhys().getSurfaceFormatsKHR(*surface);
		support.ViewportWidth = m_ViewportWidth;
		support.ViewportHeight = m_ViewportHeight;
		
		RenderContext::RecreateSwapchain(support);
		
		//auto& camera = m_App->GetCamera();
		//camera->UpdateProj(m_ViewportWidth, m_ViewportHeight, camera->GetFov());
	}

	void Renderer::Shutdown()
	{

			
			


			auto device = m_Device->GetDevice();
			device.waitIdle();


				

				std::for_each(m_Graphs.begin(),
					m_Graphs.end(),
					[](UPtr<RenderPassGraph>& graph)
					{
						graph->Destroy();
					});

			for (int i = 0; i < m_ComputeCommandBuffer.size(); i++)
			{
				m_RenderCommandBuffer[i].Free();
				m_ComputeCommandBuffer[i].Free();
				m_TransferCommandBuffer[i].Free();
			};

			Renderer::Instance()->GetCommandPoolManager()->FreePool(m_FrameCommandPool);



			m_UniformBuffers.clear();
			m_UniversalPool.reset();
			CleanUpLayouts();
			m_ImageAvailableSemaphore.clear();
			m_RenderFinishedSemaphore.clear();
			m_ComputeFinishedSemaphores.clear();
			m_InFlightFence.clear();
			m_ComputeInFlightFences.clear();
			m_ImageAvailableSemaphore.clear(); 


			m_Pipelines.clear();
			m_CommandPoolManager->Release();
			m_InstanceBuffer.reset();
			m_QuadBufferBatch.reset();
			m_QuadBufferBatchIndex.reset();
			RenderContext::Shutdown();
			


		
	}


	SPtr<Image> Renderer::GetTexture(TextureHandle handle)
	{
		return m_Textures.at(handle);
	}

	void Renderer::FillTexture(TextureHandle texture, glm::vec4& pixel, BufferHandle bufferHandle, size_t offset)
	{
		auto commandBuffer = m_TransferCommandBuffer[m_CurrentFrame];
		auto image = GetTexture(texture);
		auto buffer = m_Buffers.at(bufferHandle);
		commandBuffer.BeginTransfering();
		image->Fill(glm::vec4(-1, -1, -1, -1), commandBuffer, buffer, offset);
		commandBuffer.EndTransfering();
		commandBuffer.SubmitSingle();
	}


	
	void Renderer::CreateBuffer(BufferHandle handle, size_t size, ResourceUsage usage)
	{
		auto prop = mapBuffer(usage);
		BufferInputChunk inputBuffer;
		inputBuffer.size = size;
		inputBuffer.memoryProperties = prop.mem;
		inputBuffer.usage = prop.usage;
		m_Buffers[handle] = CreateSPtr<Buffer>(inputBuffer);
	}
	size_t Renderer::GetSize(TextureHandle handle)
	{
		return m_Textures.at(handle)->GetSize();
	}

	Renderer* Renderer::Instance()
	{
		static Renderer* renderer = new Renderer;
		return renderer;
	}
	void Renderer::UpdateBuffer(BufferHandle handle, void* data,size_t size)
	{
		auto buffer = m_Buffers.at(handle);
		buffer->SetData(data, size);
	}
	TextureHandle Renderer::GetFBTextureHandle(FrameBufferHandle fb)
	{
		auto attHandle = m_FBAttachments.at(fb)[0];
		return m_AttachmentManager.GetColorTexture(attHandle,m_CurrentFrame);
	}
	


	
	void Renderer::CreateFramebuffer(
		FrameBufferHandle handle, const std::vector<AttachmentHandle>& handles)
	{

		m_FBAttachments[handle] = handles;
		RenderPassBuilder builder;
		std::vector<int> color, depth, resolve;
		for (int i = 0; i < handles.size(); i++)
		{
			auto attachmentHandle = handles[i];
			auto& infoAttachment= m_AttachmentInfo[attachmentHandle];

			if (infoAttachment.type == AttachmentType::RESOLVE)
			{
				resolve.push_back(i);
			}

			if (infoAttachment.type == AttachmentType::COLOR)
			{
				builder.ColorOutput(attachmentHandle, m_AttachmentManager, vk::ImageLayout::eColorAttachmentOptimal);
				color.push_back(i); 
			}
			else if (infoAttachment.type == AttachmentType::DEPTH_STENCIL)
			{
				builder.DepthStencilOutput(attachmentHandle, m_AttachmentManager, vk::ImageLayout::eDepthStencilAttachmentOptimal);
				depth.push_back(i);
			}
			builder.SetLoadOp(vk::AttachmentLoadOp::eClear);
			builder.SetSaveOp(vk::AttachmentStoreOp::eStore);
			builder.SetStencilLoadOp(vk::AttachmentLoadOp::eLoad);
			builder.SetStencilSaveOp(vk::AttachmentStoreOp::eStore);
			if (infoAttachment.type == AttachmentType::COLOR)
			{
				builder.SetInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
			}
			else  if (infoAttachment.type == AttachmentType::DEPTH_STENCIL)
			{
				builder.SetInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				builder.SetFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
			}
			builder.BuildAttachmentDesc();
		}
		builder.AddSubpass(color, depth, resolve);
		vk::SubpassDependency dependency0 = SubpassDependency(VK_SUBPASS_EXTERNAL, 0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		builder.AddSubpassDependency(dependency0);

		std::vector<vk::ClearValue> clearValues;
		clearValues.reserve(handles.size());
		for (auto h : handles) {
			if (m_AttachmentInfo[h].type == AttachmentType::DEPTH_STENCIL)
				clearValues.push_back(vk::ClearDepthStencilValue{ 1.0f, 0 });
			else
				clearValues.push_back(vk::ClearColorValue(std::array<float, 4>{137.f / 255.f, 189.f / 255.f, 199.f / 255.f, 1}));
		}

		vk::Extent2D extent = { static_cast<uint32_t>(m_AttachmentInfo[handles[0]].width),static_cast<uint32_t>(m_AttachmentInfo[handles[0]].height) };
		auto renderPass = builder.Build(m_AttachmentManager, RenderContext::GetFrameAmount(), extent, clearValues);
		//auto renderPassHandle = g_RenderPassAllocator.GetId();
		m_RenderPasses[handle] = renderPass;
		m_Framebuffers[handle] = _CreateFramebuffer(renderPass.m_RenderPass,extent.width,extent.height, builder, handles);
	}

	void Renderer::CreateAttachment(AttachmentHandle handle, AttachmentInfo_ info)
	{
		auto usage = map(info.hints);
		m_AttachmentInfo[handle] = info;
		switch (info.type)	
		{
		case AttachmentType::COLOR:
		{
			auto mem = DeriveAttachmentMemoryPrefs(info.hints);
			m_AttachmentManager.CreateColor(handle,map(info.format),info.width,info.height,map(info.samples),
				usage | vk::ImageUsageFlagBits::eColorAttachment, RenderContext::GetFrameAmount(), mem);
			break;

		}
		case AttachmentType::DEPTH_STENCIL:
		{
			m_AttachmentManager.CreateDepthStencil(handle, info.width, info.height, map(info.samples), usage | vk::ImageUsageFlagBits::eDepthStencilAttachment, RenderContext::GetFrameAmount());
			break;
		}
		case AttachmentType::RESOLVE:
		{

			break;
		}
		default:
			break;
		}
		
	}
	void Renderer::CompileShader(std::string_view path)
	{
		m_Compiler.Compile(std::filesystem::path{path});
	}
	void Renderer::LinkShaders(ProgramHandle handle ,uint8_t shaderAmount)
	{
		m_Compiler.Link(handle, shaderAmount);
	}

	void Renderer::CreateEmptyTexture(TextureHandle handle, const CreateEmptyTextureCmd& cmd)
	{
		auto prop = mapImage(cmd.usage);
		auto viewType = pickViewType(cmd.width, cmd.height, cmd.layers, cmd.cube);
		auto image  = Image::CreateEmptyImage(
			cmd.width,
			cmd.height,
			map(cmd.format),
			prop.usage,
			cmd.mipLevels,
			map(cmd.samples),
			map(cmd.minFilter),
			map(cmd.magFilter),
			static_cast<int>(cmd.layers),
			viewType
		);
		m_Textures[handle] = image;
	}

	void Renderer::CreateEmptyMipMapsAsImages(TextureHandle handle, std::vector <TextureHandle>& handles)
	{
		auto image = m_Textures.at(handle);
		auto mipImages = image->GenerateEmptyMipmapsAsImages(handles.size());

		for (auto i = 0; i < handles.size(); i++)
		{
			m_Textures[handles[i]] = mipImages[i];
		}
	}

	void Renderer::CreateTexture(TextureHandle handle, std::string_view path)
	{
		auto image = Image::CreateImage(BASE_RES_PATH + std::string{path});
		m_Textures[handle] = image;
	}

	

	void Renderer::CreateVertexBuffer(Memory& mem, VertexBufferHandle vertHandle, ResourceUsage usage)
	{
		auto prop = mapBuffer(usage);
		BufferInputChunk input;
		input.size = mem.size;
		input.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		input.memoryProperties = prop.mem;
		auto& buffer = m_VertexBuffers[vertHandle] = CreateSPtr<Buffer>(input);
		SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(mem.size);

		m_TransferCommandBuffer[0].BeginTransfering();
		m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), buffer.get(), mem.data, mem.size);
		m_TransferCommandBuffer[0].EndTransfering();
		m_TransferCommandBuffer[0].SubmitSingle();
	}

	void Renderer::CreateIndexBuffer(Memory& mem, IndexBufferHandle indexHandle)
	{
		BufferInputChunk input;
		input.size = mem.size;
		input.usage = vk::BufferUsageFlagBits::eIndexBuffer |  vk::BufferUsageFlagBits::eTransferDst;
		input.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		auto indexAmount = mem.size / 4;
		auto& buffer = m_IndexBuffers[indexHandle] = CreateSPtr<IndexBuffer>(input, indexAmount, vk::IndexType::eUint32);

		SPtr<Buffer> stagingBuffer = Buffer::CreateStagingBuffer(mem.size);

		m_TransferCommandBuffer[0].BeginTransfering();
		m_TransferCommandBuffer[0].Transfer(stagingBuffer.get(), buffer.get(), mem.data, mem.size);
		m_TransferCommandBuffer[0].EndTransfering();
		m_TransferCommandBuffer[0].SubmitSingle();

	}
	void Renderer::UpdateRegionWithImage(Memory& mem, size_t width, size_t height, TextureHandle image, vk::Offset3D offset, int layer)
	{
		auto imageToUpdate = m_Textures.at(image);
		Image::UpdateRegionWithImage(mem, width, height,
			imageToUpdate
			, offset,
			layer);
	}
	void Renderer::AddFramebuffers(FrameBufferHandle handle, std::vector<vk::Framebuffer>& framebuffers)
	{
		if (m_Framebuffers.find(handle) != m_Framebuffers.end())
		{
			Log::GetLog()->error("Adding existing frame buffer");
		}
		m_Framebuffers[handle] = framebuffers;
	}

	vk::Pipeline Renderer::GetPipeline(const PipelineKey& key, 
		std::array<VertexBinding, Item::MAX_VERTEX_BINDING>& bindings,
		int bindingAmount)
	{
		if (m_Pipelines.find(key) != m_Pipelines.end())
			return m_Pipelines.at(key);

		/*
		struct PipelineKey
		{
			ProgramHandle program;
			RenderState rs;
			PipelineLayoutKey layout;
			RenderPassHandle_ renderPass;
		};
		*/

		PipelineBuilder builder;
		auto& rs = key.rs;
		builder.EnableStencilTest(rs.stencilTest);
		builder.SetDepthTest(true);
		builder.EnableBlend(rs.blend->enabled);
		auto& renderPass = m_RenderPasses.at(key.fb);

		builder.SetRenderPass(renderPass.m_RenderPass);
		builder.AddExtent(renderPass.m_Extent);

		builder.WriteToDepthBuffer(true);
		builder.SetSamples(renderPass.samples);
		auto shaderMeta = m_Compiler.m_Programs.at(key.program);
		for (auto& stage : shaderMeta.stages)
		{
			builder.AddShader(stage.module, map(stage.stage));
		}

		builder.SetTopology(map(rs.topology));
		builder.SetPolygoneMode(map(rs.polygon));
		{
			std::vector<vk::DescriptorSetLayout> layouts;
			for (DescriptorLayoutKey& key: shaderMeta.descriptorKey)
			{
				auto& layout = m_DescriptorLayout.at(key);
				layouts.push_back(layout);

			}
			builder.AddDescriptorSetLayouts(layouts);
		}

		for (int i = 0; i < bindingAmount; i++)
		{
			VertexBinding& binding = bindings[i];
			
			const auto& vertexLayout = GetVertexLayout(binding.LayoutHandle);

			struct Vertex
			{
				glm::vec3 Position;
				//glm::vec4 Color;
				glm::vec2 UV;
				//alignas(4)
				//float textureID;
			};
			auto size = sizeof Vertex;
			auto vInputBindDescription = VertexBindingDescription(0, vertexLayout.m_CurrentOffset, vk::VertexInputRate::eVertex);
			
			builder.AddBindingDescription(vInputBindDescription);

			for (int ii =0; ii < vertexLayout.m_Elements.size(); ii++)
			{
				auto& element = vertexLayout.m_Elements[ii];
				auto desc = VertexInputAttributeDescription(i, ii, map(element.type), element.offset);
				builder.AddAttributeDescription(desc);
			}
		}
		
		

		// for now just 1 - potentially we can merge render passes into one
		builder.SetSubpassIndex(0);



		//void StencilTestOp(vk::CompareOp op, vk::StencilOp fail, vk::StencilOp pass, vk::StencilOp //depthFailOp);
		//void SetControlPoints(int amountPoints);

		builder.AddPipelineLayout(m_PipelineLayout.at(key.layout));
		m_Pipelines[key] = builder.Build();
		return m_Pipelines[key];
	}
	


	struct ProfileResult
		{
			std::string name;
			long long start, end;
		};
	class Profiler
		{
		public:
			void BeginSession(const std::string& name, const std::string& filepath)
			{
				m_outputStream.open(filepath);
				WriteHeader();
			}

			void WriteProfile(const ProfileResult& result)
			{
				if (m_profileCount++ > 0)
					m_outputStream << ",";

				std::string name = result.name;
				std::replace(name.begin(), name.end(), '"', '\'');

				m_outputStream << "{";
				m_outputStream << "\"cat\":\"function\",";
				m_outputStream << "\"dur\":" << (result.end - result.start) << ',';
				m_outputStream << "\"name\":\"" << name << "\",";
				m_outputStream << "\"ph\":\"X\",";
				m_outputStream << "\"pid\":0,";
				m_outputStream << "\"tid\":0,";
				m_outputStream << "\"ts\":" << result.start;
				m_outputStream << "}";

				m_outputStream.flush();
			}

			void WriteHeader()
			{
				m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
				m_outputStream.flush();
			}

			void WriteFooter()
			{
				m_outputStream << "]}";
				m_outputStream.flush();
			}

			static Profiler& Get()
			{
				static Profiler instance;
				return instance;
			}

			void EndSession()
			{
				WriteFooter();
				m_outputStream.close();
				m_profileCount = 0;
			}
		private:
			std::ofstream m_outputStream;
			int m_profileCount;
		};



	class ProfileTimer
		{
		public:

			ProfileTimer() : m_name{ "no name" }
			{
				m_startPoint = std::chrono::high_resolution_clock::now();
			}

			ProfileTimer(const char* name) : m_name{ name }
			{
				m_startPoint = std::chrono::high_resolution_clock::now();
			}
			~ProfileTimer()
			{
				Stop();
			}
			void Stop()
			{
				auto endPoint = std::chrono::high_resolution_clock::now();

				auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startPoint)
					.time_since_epoch().count();
				auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint)
					.time_since_epoch().count();
				auto duration = end - start;
				double ms = duration * 0.001;

				Profiler::Get().WriteProfile({ m_name, start, end });
			}


			double Elapsed()
			{
				auto endPoint = std::chrono::high_resolution_clock::now();

				auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startPoint)
					.time_since_epoch().count();
				auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint)
					.time_since_epoch().count();
				auto duration = end - start;
				double ms = duration * 0.001;
				return ms;
			}
		private:
			std::chrono::time_point<std::chrono::high_resolution_clock> m_startPoint;
			std::string m_name;
		};


	class Timer
		{
		public:

			Timer()
			{
				m_startPoint = std::chrono::high_resolution_clock::now();
			}

			double Elapsed()
			{
				auto endPoint = std::chrono::high_resolution_clock::now();

				auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startPoint)
					.time_since_epoch().count();
				auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint)
					.time_since_epoch().count();
				auto duration = end - start;
				double ms = duration * 0.001;
				return ms;
			}
		private:
			std::chrono::time_point<std::chrono::high_resolution_clock> m_startPoint;

		};

	

	vk::Pipeline Renderer::GetComputePipeline(PipelineKey& key)
	{
		if (m_Pipelines.find(key) != m_Pipelines.end())
			return m_Pipelines.at(key);
		
		auto device = RenderContext::GetDevice();
		auto layout = m_PipelineLayout.at(key.layout);

		vk::PipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.flags = vk::PipelineShaderStageCreateFlags();
		computeShaderStageInfo.stage = vk::ShaderStageFlagBits::eCompute;
		computeShaderStageInfo.module = m_Compiler.m_Programs.at(key.program).stages[0].module;
		computeShaderStageInfo.pName = "main";
		
		vk::ComputePipelineCreateInfo pipelineInfo{};
		
		pipelineInfo.sType = vk::StructureType::eComputePipelineCreateInfo;
		pipelineInfo.layout = layout;
		pipelineInfo.stage = computeShaderStageInfo;
		try
		{
			m_Pipelines[key] = device->GetDevice().createComputePipeline(nullptr, pipelineInfo).value;
			Log::GetLog()->info("Compute pipeline is Created!");
			return m_Pipelines[key];
		}
		catch (vk::SystemError err)
		{
			Log::GetLog()->error("Failed to create Pipeline");
			return {};
		}

	}


	void Renderer::BindDescriptors()
	{

	}

	void Renderer::RenderFrame(Frame* render, float deltaTime)
	{
		Timer timer;
		if (render->CurrentRenderItemIndex == 0) return;
		
		
		
		uint32_t imageIndex = 0;
		auto swapchain = RenderContext::GetSwapchain();
		m_Device->GetDevice().acquireNextImageKHR(swapchain->m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore[m_CurrentFrame].GetSemaphore(), nullptr, &imageIndex);

		auto& currentFence = m_InFlightFence[m_CurrentFrame];
		Renderer::Instance()->Wait(currentFence.GetFence());
		Renderer::Instance()->Reset(currentFence.GetFence());


		auto& cmd = m_RenderCommandBuffer[m_CurrentFrame];
		cmd.BeginRendering();


	
		for (int i = 0; i < render->CurrentRenderItemIndex; i++)
		{

			auto item = (render->m_renderItem + i);
			if (item->Type == ItemType::COMPUTE)
			{
				auto& computeItem = *item;
				auto vkCmd = cmd.GetCommandBuffer();
				auto& meta = m_Compiler.m_Programs.at(computeItem.Program);
				std::vector<DescriptorLayoutKey>& keys = meta.descriptorKey;
				PipelineKey key = { computeItem.Program,{},keys,{} };
				vk::Pipeline pipeline = GetComputePipeline(key);
				vk::PipelineLayout layout = m_PipelineLayout.at(key.layout);
				for (int ii = 0; ii < keys.size(); ii++)
				{
					auto k = keys.at(ii);
					if (m_DescriptorSet.find(k) == m_DescriptorSet.end())
					{
						AllocateSets(RenderContext::GetFrameAmount(), k);
					}
				}

				for (int ii = 0; ii < computeItem.Bindings.currentResBinding; ii++)
				{

					auto& bind = computeItem.Bindings.ResBindings[ii];
					if (bind.dirty)
					{
						// update descriptor
						auto setNumber = meta.uniforms.at(bind.uniform).first;
						auto bindNumber = meta.uniforms.at(bind.uniform).second;
						auto& k = *std::find_if(keys.begin(), keys.end(), [=](auto key) {return key.set == setNumber; });
						std::vector<vk::DescriptorImageInfo> descirptors;
						if (bind.kind == ResourceType::CombinedSampler || bind.kind == ResourceType::StorageImage)
						{
							for (auto handle : bind.handles)
							{
								if (!handle.Valid()) break;
								auto image = m_Textures.at(handle);
								if (bind.kind == ResourceType::CombinedSampler)
								{
									cmd.ChangeImageLayout(image.get(), image->GetLayout(), vk::ImageLayout::eShaderReadOnlyOptimal, image->m_MipMapLevels);
								}
								else if (bind.kind == ResourceType::StorageImage)
								{
									cmd.ChangeImageLayout(image.get(), image->GetLayout(), vk::ImageLayout::eGeneral, image->m_MipMapLevels);
								}
							
								vk::DescriptorImageInfo imageDescriptor1;
								imageDescriptor1.imageLayout = image->GetLayout();
								imageDescriptor1.imageView = image->GetImageView();
								imageDescriptor1.sampler = image->GetSampler();
								descirptors.push_back(imageDescriptor1);

							}

							m_Device->UpdateDescriptorSet(m_DescriptorSet.at(k)[m_CurrentFrame], bindNumber, descirptors, bind.kind);
						}
						else if (bind.kind == ResourceType::StorageBuffer)
						{
							for (auto handle : bind.buffers)
							{
								if (!handle.Valid()) break;
								auto buffer = m_Buffers.at(handle);
								m_Device->UpdateDescriptorSet(
									m_DescriptorSet.at(k)[m_CurrentFrame], 1, 1, *buffer, ResourceType::StorageBuffer);
							}
						}
						bind.dirty = false;
					}
				}

				for (int ii = 0; ii < keys.size(); ii++)
				{
					auto k = keys.at(ii);
					auto& descSet = m_DescriptorSet.at(k);
					vkCmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout, ii, descSet[m_CurrentFrame], nullptr);

				}

				vkCmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
				vkCmd.dispatch(computeItem.GroupCount.x, computeItem.GroupCount.y, computeItem.GroupCount.z);


				computeItem.Bindings.Reset();

			}
			else if ( item->Type == ItemType::RENDER)
			{
				auto& renderItem = *item;
				View& view = render->Views[renderItem.View];
				assert(renderItem.Program.Valid());
				auto& meta = m_Compiler.m_Programs.at(renderItem.Program);
				if (!view.Fbh.Valid())
				{
					view.Fbh = DEFAULT_FRAME_BUFFER;
					//struct PipelineKey
					//{
					//	ProgramHandle program;
					//	RenderState rs;
					//	PipelineLayoutKey layout;
					//	RenderPassHandle_ renderPass;
					//};

				}


				// get pipeline
				std::vector<DescriptorLayoutKey>& keys = meta.descriptorKey;

				PipelineKey key = { renderItem.Program,renderItem.State,keys,view.Fbh };

				vk::Pipeline pipeline = GetPipeline(key, renderItem.VertexBindings, renderItem.Bindings.currentBinding);
				vk::PipelineLayout layout = m_PipelineLayout.at(key.layout);


				for (int ii = 0; ii < keys.size(); ii++)
				{
					auto k = keys.at(ii);
					if (m_DescriptorSet.find(k) == m_DescriptorSet.end())
					{
						AllocateSets(RenderContext::GetFrameAmount(), k);
					}

				}

				// updating uniforms
				//struct ResourceBinding
				//{
				//	std::string uniform;
				//	TextureHandle handle;
				//	bool dirty;
				//};
				for (int ii = 0; ii < renderItem.Bindings.currentResBinding; ii++)
				{
					auto& bind = renderItem.Bindings.ResBindings[ii];
					if (bind.dirty)
					{
						// update descriptor
						auto setNumber = meta.uniforms.at(bind.uniform).first;
						auto bindNumber = meta.uniforms.at(bind.uniform).second;
						auto& k = *std::find_if(keys.begin(), keys.end(), [=](auto key) {return key.set == setNumber; });

						auto image = m_Textures.at(bind.handles[0]);


						cmd.ChangeImageLayout(image.get(), image->GetLayout(), vk::ImageLayout::eShaderReadOnlyOptimal, image->m_MipMapLevels);

						vk::DescriptorImageInfo imageDescriptor1;
						assert(image->GetLayout() == vk::ImageLayout::eShaderReadOnlyOptimal);
						imageDescriptor1.imageLayout = image->GetLayout();
						imageDescriptor1.imageView = image->GetImageView();
						imageDescriptor1.sampler = image->GetSampler();

						m_Device->UpdateDescriptorSet(m_DescriptorSet.at(k)[m_CurrentFrame], bindNumber, 1, imageDescriptor1, ResourceType::CombinedSampler);
						bind.dirty = false;



					}
				}

				auto& renderPass = m_RenderPasses.at(key.fb);
				auto frameBuffer = m_Framebuffers.at(key.fb)[imageIndex];


				auto& test = m_FBAttachments[key.fb];

				for (auto handle : test)
				{
					auto texHandle = m_AttachmentManager.GetColorTexture(handle, imageIndex);
					auto image = m_Textures.at(texHandle);
					cmd.ChangeImageLayout(image.get(), image->GetLayout(), vk::ImageLayout::eColorAttachmentOptimal, image->m_MipMapLevels);
				}


				cmd.BeginRenderPass(renderPass.m_RenderPass, frameBuffer, renderPass.m_Extent, renderPass.m_ClearValues);
				auto vkCmd = cmd.GetCommandBuffer();
				vkCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

				UpdateUniformBuffer(view.Proj, view.View, m_App->GetExeTime());


				for (int ii = 0; ii < keys.size(); ii++)
				{
					auto k = keys.at(ii);
					auto& descSet = m_DescriptorSet.at(k);
					vkCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, ii, descSet[m_CurrentFrame], nullptr);

				}
				vk::Viewport viewport;
				viewport.x = view.Rect[0];
				viewport.y = view.Rect[1];
				viewport.width = view.Rect[2];
				viewport.height = view.Rect[3];
				viewport.minDepth = 0;
				viewport.maxDepth = 1;
				vk::Rect2D scissors;
				scissors.offset = vk::Offset2D{ static_cast<int32_t>(view.Rect[0]),static_cast<int32_t>(view.Rect[1]) };
				scissors.extent = vk::Extent2D{ static_cast<uint32_t>(view.Rect[2]),static_cast<uint32_t>(view.Rect[3]) };
				vkCmd.setViewport(0, 1, &viewport);
				vkCmd.setScissor(0, 1, &scissors);

				std::vector<vk::Buffer> vertexBuffers;
				vertexBuffers.reserve(renderItem.Bindings.currentBinding);
				std::vector<vk::DeviceSize> offsets(renderItem.Bindings.currentBinding, 0);

				for (auto ii = 0; ii < renderItem.Bindings.currentBinding; ii++)
				{
					auto& buffer = m_VertexBuffers.at(renderItem.VertexBindings.at(ii).VertexHandle)->GetBuffer();
					vertexBuffers.push_back(buffer);
				}
				if (vertexBuffers.size() > 0)
					vkCmd.bindVertexBuffers(0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), offsets.data());
				if (renderItem.IndexBuffer.Valid())
				{
					auto buffer = m_IndexBuffers.at(renderItem.IndexBuffer);
					vkCmd.bindIndexBuffer(buffer->GetBuffer(), vk::DeviceSize{ 0 }, buffer->GetIndexType());
					vkCmd.drawIndexed(buffer->GetIndexAmount(), 1, 0, 0, 0);
				}
				else
				{
					// draw without index
					vkCmd.draw(6, 1, 0, 0);
				}


				cmd.EndRenderPass();
				renderItem.Bindings.Reset();
			}
		}

		cmd.EndRendering();
		

		auto waitSemaphore = m_ImageAvailableSemaphore[m_CurrentFrame].GetSemaphore();


		std::vector<vk::Semaphore> signal = { m_RenderFinishedSemaphore[m_CurrentFrame].GetSemaphore(), m_TimelineSemaphore[0].GetSemaphore() };

		vk::TimelineSemaphoreSubmitInfo info{};
		info.signalSemaphoreValueCount = 2;
		size_t value = m_FrameNumber + 1;
		uint64_t    signalVals[2] = { 0,  value };  
		info.pSignalSemaphoreValues = signalVals;


		signal.pop_back();
		auto fence = currentFence.GetFence();
		cmd.Submit({waitSemaphore}, signal, &fence);
		
		
		vk::Semaphore waitSemaphores[] = { signal[0]};
		vk::PresentInfoKHR presentInfo = {};

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemaphores;

		vk::SwapchainKHR swapChains[] = { swapchain->m_Swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;
		vk::Result present;
		try {
			ZoneScopedN("Presenting");
			present = m_Device->GetPresentQueue().presentKHR(presentInfo);
		}
		catch (vk::OutOfDateKHRError error) {
			present = vk::Result::eErrorOutOfDateKHR;
		}
		if (present == vk::Result::eErrorOutOfDateKHR)
		{
			ZoneScopedN("Recreating swapchain");
			RecreateSwapchain();
		}
		//m_Device->GetDevice().waitIdle();
		
		//std::cout << timer.Elapsed() << std::endl;
		FrameMark;
	}

	void Renderer::Wait(const vk::Fence& fence)
	{
		m_Device->GetDevice().waitForFences(fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	}
	void Renderer::Reset(const vk::Fence& fence)
	{
		m_Device->GetDevice().resetFences(fence);
	}

	
	void Renderer::Flush(std::vector<vk::CommandBuffer> commandBuffers)
	{
		assert(false);
	}
	void Renderer::UpdateUniformBuffer(const glm::mat4& proj, const glm::mat4& view, float time)
	{
		UniformBufferObject ubo{};

		ubo.view = view;
		ubo.proj = proj;
		ubo.time = time;
		memcpy(m_UniformBuffersMapped[m_CurrentFrame], &ubo, sizeof(ubo));
	}

	void Renderer::BeginFrame(Frame* frame)
	{
		m_FrameNumber = frame->FrameNumber;
	}
	void Renderer::EndFrame(Frame* frame)
	{
		m_CurrentFrame = (m_CurrentFrame + 1) % RenderContext::GetFrameAmount();
	}

	static vk::DescriptorSetLayout CreateDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding>& bindings)
	{


		auto device = RenderContext::GetDevice();

		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits();
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		try
		{
			return device->GetDevice().createDescriptorSetLayout(layoutInfo);
		}
		catch (vk::SystemError err)
		{

			Log::GetLog()->error("Failed to create Descriptor Set Layout");
			return nullptr;
		}
	}


	void Renderer::CreatePipelineLayout(PipelineLayoutKey& key)
	{
		std::vector<vk::DescriptorSetLayout> layouts;
		for (auto k : key.layoutKeys)
		{
			layouts.push_back(m_DescriptorLayout.at(k));
		}
		m_PipelineLayout[key] = MakePipelineLayout(RenderContext::GetDevice()->GetDevice(), layouts);
	}

	vk::DescriptorSetLayout Renderer::CreateDescriptorLayout(const DescriptorLayoutKey& key)
	{

		
		auto& bindings = key.bindings;
		std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
		for (auto& v : bindings)
		{
			vkBindings.push_back(DescriptorBindingDescription(v.binding, map(v.kind), mapAccess(v.access), v.count));
		}

		m_DescriptorLayout[key] = CreateDescriptorSetLayout(vkBindings);
		return m_DescriptorLayout[key];
	}
	
	void Renderer::CopyBufferToPtr(SPtr<Buffer> buffer, void* data, size_t offset)
	{
		auto bufferSize = buffer->GetSize();
		auto ptr = m_Device->GetDevice().mapMemory(buffer->GetMemory(), (uint64_t)0, buffer->GetSize());
	
		std::memcpy(data, ptr, bufferSize);
		m_Device->GetDevice().unmapMemory(buffer->GetMemory());
	}
	void Renderer::CopyImageToBuffer(SPtr<Image> image, SPtr<Buffer> buffer)
	{
		auto& transferBuffer = Renderer::Instance()->GetTransferCommandBuffer(m_CurrentFrame);

		auto value = m_FrameNumber;
		vk::TimelineSemaphoreSubmitInfo info{
			 1, &(value),
			 0, nullptr,
		};
		Fence fence;
		Renderer::Instance()->Reset(fence.GetFence());
		transferBuffer.BeginTransfering();
		transferBuffer.CopyImageToBuffer(image, buffer);
		transferBuffer.EndTransfering();
		auto vkFence = fence.GetFence();
		transferBuffer.Submit({}, {}, &vkFence);
		Renderer::Instance()->Wait(fence.GetFence());
			
	}


	

	Renderer::~Renderer()
	{
		
	}



	void Renderer::CreateInstance()
	{
		InstanceInfo info;
		uint32_t version;
		vkEnumerateInstanceVersion(&version);
		info.vkInfo = vk::ApplicationInfo
		{
			"Voidstar app",
			version,
			"Voidstar",
			version,
			version,
		};
		info.extensionCount = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&info.extensionCount);
		for (auto i = 0; i < info.extensionCount; i++) {
			info.extensions.emplace_back(extensions[i]);
		}

		info.extensions.push_back("VK_EXT_debug_utils");
		//tracy
		info.extensions.push_back("VK_EXT_calibrated_timestamps");

		info.layers.push_back("VK_LAYER_KHRONOS_validation");
		RenderContext::CreateInstance(info);
		m_Instance = RenderContext::GetInstance();
		
	}
	
}
