#pragma once
#include "ResourceCommandBuffer.h"
#include <string_view>
#include "Core.h"
#include "Rendering/ShaderType.h"
#include "InitParams.h"
#include "Window.h"
#include "VertexLayout.h"
#include "Handle.h"
#include "Util.h"
#include "glm.hpp"
#include <unordered_map>
#include <cstdint>
#include <array>
#include <vector>
#include <variant>
#include "Memory.h"
#include "Rendering/Generation.h"

namespace Voidstar
{
	const size_t MAX_OBJECTS = 255;

	enum class ResourceType : uint8_t
	{
		UniformBuffer,
		StorageBuffer,

		VertexBuffer,

		StorageImage,
		CombinedSampler,
	};

	struct ProgramTag {};
	struct ShaderTag {};
	struct VertexBufferTag {};
	struct IndexBufferTag {};
	struct FramebufferTag {};
	struct VertexLayoutTag {};
	struct BufferTag {};
	struct TextureTag {};
	struct UniformTag {};
	struct AttachmentTag {};

	using ProgramHandle = Handle<ProgramTag>;
	using ShaderHandle = Handle<ShaderTag>;
	using VertexBufferHandle = Handle<VertexBufferTag>;
	using IndexBufferHandle = Handle<IndexBufferTag>;
	using FrameBufferHandle = Handle<FramebufferTag>;
	using AttachmentHandle = Handle<AttachmentTag>;
	using VertexLayoutHandle = Handle<VertexLayoutTag>;
	using BufferHandle = Handle<BufferTag>;
	using TextureHandle = Handle<TextureTag>;
	using UniformHandle = Handle<UniformTag>;
	using PassID = uint16_t;


	struct RenderPassTag {};
	using RenderPassHandle_ = Handle<RenderPassTag>;
	template<
		class Key,
		class T,
		class Hash = std::hash<Key>,
		class KeyEqual = std::equal_to<Key>,
		class Allocator = std::allocator<std::pair<const Key, T>>
	>
	using Map = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;
}

namespace std {
	template <class Tag>
	struct hash<Voidstar::Handle<Tag>> {
		size_t operator()(const Voidstar::Handle<Tag>& h) const noexcept {
			return std::hash<uint16_t>{}(h.idx);
		}
	};
}
namespace Voidstar
{

	

	enum class ResourceUsage : uint32_t {
		None = 0,

		// GPU reads
		Sampled = 1 << 0,  // sampled image / sampled buffer (TEX)
		StorageRead = 1 << 1,  // SSBO/Storage read
		StorageWrite = 1 << 2,  // SSBO/Storage write

		// Attachments
		ColorTarget = 1 << 3,  // color attachment
		DepthStencil = 1 << 4,  // depth-stencil attachment

		// Copies
		TransferSrc = 1 << 5,
		TransferDst = 1 << 6,

		// Geometry / draw
		Vertex = 1 << 7,
		Index = 1 << 8,
		Indirect = 1 << 9,
		Uniform = 1 << 10, // UBO / CBV

		// CPU access hints (high level; maps to memory properties)
		Upload = 1 << 11, // CPU→GPU frequent writes
		Readback = 1 << 12, // GPU→CPU reads

		// Image shape hints (optional helpers)
		Cube = 1 << 13,
		Mipmapped = 1 << 14,
		
	};

	inline ResourceUsage operator|(ResourceUsage a, ResourceUsage b) {
		return static_cast<ResourceUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline bool HasFlag(ResourceUsage u, ResourceUsage f) {
		return (static_cast<uint32_t>(u) & static_cast<uint32_t>(f)) != 0;
	}
	enum class FilterMode { Nearest, Linear };

	enum class TextureFormat {
		Unknown,

		// 8-bit
		R8_UNORM, R8_SNORM, R8_UINT, R8_SINT, R8_SRGB,
		RG8_UNORM, RG8_SNORM, RG8_UINT, RG8_SINT,
		RGBA8_UNORM, RGBA8_SNORM, RGBA8_UINT, RGBA8_SINT, RGBA8_SRGB,
		BGRA8_UNORM, BGRA8_SRGB,

		// packed / special
		RGB10A2_UNORM,
		R11G11B10_UFLOAT,

		// 16-bit
		R16_UNORM, R16_SNORM, R16_UINT, R16_SINT, R16_SFLOAT,
		RG16_UNORM, RG16_SNORM, RG16_UINT, RG16_SINT, RG16_SFLOAT,
		RGBA16_UNORM, RGBA16_SNORM, RGBA16_UINT, RGBA16_SINT, RGBA16_SFLOAT,

		// 32-bit
		R32_UINT, R32_SINT, R32_SFLOAT,
		RG32_UINT, RG32_SINT, RG32_SFLOAT,
		RGB32_UINT, RGB32_SINT, RGB32_SFLOAT,
		RGBA32_UINT, RGBA32_SINT, RGBA32_SFLOAT,

		// depth/stencil
		D16_UNORM,
		X8_D24_UNORM,
		D32_SFLOAT,
		D24_UNORM_S8_UINT,
		D32_SFLOAT_S8_UINT,
		S8_UINT,

		// compressed (BC)
		BC1_RGBA_UNORM, BC1_RGBA_SRGB,
		BC3_RGBA_UNORM, BC3_RGBA_SRGB,
		BC4_R_UNORM, BC4_R_SNORM,
		BC5_RG_UNORM, BC5_RG_SNORM,
		BC7_RGBA_UNORM, BC7_RGBA_SRGB,
	};



	enum class SampleCount
	{
		e1 = 1,
		e2 = 2,
		e4 = 4,
		e8 = 8,
		e16 = 16,
		e32 = 32,
		e64 = 64,
	};
	
	enum class AttachmentType
	{
		COLOR,
		DEPTH_STENCIL,
		RESOLVE
	};

	



	enum class AttachmentHint : uint32_t {
		None = 0,
		Transient = 1 << 0,  // don’t preserve; prefer lazily allocated if available
		SampledLater = 1 << 1,  // used as sampled input in a later pass
		Storage = 1 << 2,  // used as storage image (compute or raster UAV)
		ResolveSrc = 1 << 3,  // multisampled source to be resolved
		ResolveDst = 1 << 4,  // single-sample resolve target
		Readback = 1 << 5,  // will be copied to a staging buffer/image
		Presentable = 1 << 6,  // swapchain image (special case; you won’t create it)
		InputAttachment = 1 << 7,  // subpass input attachment
	};
	inline AttachmentHint operator|(AttachmentHint a, AttachmentHint b) {
		return static_cast<AttachmentHint>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline bool HasFlag(AttachmentHint h, AttachmentHint bit) {
		return (static_cast<uint32_t>(h) & static_cast<uint32_t>(bit)) != 0;
	}


	struct CreateEmptyTextureCmd
	{
		int32_t      width = 1;
		int32_t      height = 1;
		TextureFormat format{};
		ResourceUsage usage{};
		int mipLevels = 1;
		SampleCount   samples = SampleCount::e1;
		FilterMode    minFilter = FilterMode::Linear;
		FilterMode    magFilter = FilterMode::Linear;
		uint32_t      layers = 1;
		bool          cube = false;
	};



	struct AttachmentInfo_
	{
		AttachmentType type;
		TextureFormat format;
		int width;
		int height;
		SampleCount samples;
		AttachmentHint hints;
	};

	

	struct VertexBinding
	{
		VertexStreamMode Mode;
		VertexBufferHandle VertexHandle;
		VertexLayoutHandle LayoutHandle;
	};


	enum class Topology { TriList, TriStrip, LineList, LineStrip, Point };
	enum class Culling { None, Back, Front };
	enum class Polygon { Fill, Line, Point };

	enum class BlendOp { Add, Subtract, ReverseSubtract, Min, Max };
	enum class CompareOp { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };
	enum class StencilOp { Keep, Zero, Replace, IncrClamp, DecrClamp, Invert, IncrWrap, DecrWrap };

	enum class BlendFactor {
		Zero, One,
		SrcColor, OneMinusSrcColor,
		DstColor, OneMinusDstColor,
		SrcAlpha, OneMinusSrcAlpha,
		DstAlpha, OneMinusDstAlpha,
		ConstColor, OneMinusConstColor,
		ConstAlpha, OneMinusConstAlpha,
		SrcAlphaSaturate
	};

	struct BlendMode {
		bool        enabled = true;
		BlendFactor srcColor = BlendFactor::SrcAlpha; 
		BlendFactor dstColor = BlendFactor::OneMinusSrcAlpha; 
		BlendOp     colorOp = BlendOp::Add;
		BlendFactor srcAlpha = BlendFactor::One;
		BlendFactor dstAlpha = BlendFactor::OneMinusSrcAlpha;
		BlendOp     alphaOp = BlendOp::Add;

		bool operator==(const BlendMode& o) const {
			return enabled == o.enabled
				&& srcColor == o.srcColor
				&& dstColor == o.dstColor
				&& colorOp == o.colorOp
				&& srcAlpha == o.srcAlpha
				&& dstAlpha == o.dstAlpha
				&& alphaOp == o.alphaOp;
		}
	};


	struct BlendModeHash {
		std::size_t operator()(const BlendMode& b) const noexcept {
			std::size_t h = 0;
			util::hash_combine(h, b.enabled);
			util::hash_combine(h, util::hash_enum(b.srcColor));
			util::hash_combine(h, util::hash_enum(b.dstColor));
			util::hash_combine(h, util::hash_enum(b.colorOp));
			util::hash_combine(h, util::hash_enum(b.srcAlpha));
			util::hash_combine(h, util::hash_enum(b.dstAlpha));
			util::hash_combine(h, util::hash_enum(b.alphaOp));
			return h;
		}
	};

	struct RenderState {
		
		Topology topology = Topology::TriList;
		Polygon  polygon = Polygon::Fill;
		Culling     cull = Culling::None;		
		bool      depthTest = true;
		bool      depthWrite = true;
		bool	  stencilTest = false;
		CompareOp depthFunc = CompareOp::LessEqual;
		// depends on amount of colour targets
		BlendMode blend[1]{};
		//StencilMode stencil{};

		bool operator==(const RenderState& o) const noexcept {
			return topology == o.topology
				&& polygon == o.polygon
				&& cull == o.cull
				&& depthTest == o.depthTest
				&& depthWrite == o.depthWrite
				&& depthFunc == o.depthFunc
				&& blend[0] == o.blend[0];
		}
	};


	struct RenderStateHash {
		std::size_t operator()(const RenderState& rs) const noexcept {
			std::size_t h = 0;
			util::hash_combine(h, util::hash_enum(rs.topology));
			util::hash_combine(h, util::hash_enum(rs.polygon));
			util::hash_combine(h, util::hash_enum(rs.cull));
			util::hash_combine(h, rs.depthTest);
			util::hash_combine(h, rs.depthWrite);
			util::hash_combine(h, util::hash_enum(rs.depthFunc));
			// If you later support N util::color targets, iterate N and combine each.
			util::hash_combine(h, BlendModeHash{}(rs.blend[0]));
			return h;
		}
	};


	struct ResourceBinding
	{
		std::string uniform;
		std::array<TextureHandle, 12> handles;
		std::array<BufferHandle,  12> buffers;
		int currentHandle = 0;
		int currentHandle2 = 0;
		bool dirty;
		ResourceType kind;
	};

	struct ResourceBindings
	{
		static constexpr int MAX_RES_BINDING = 10;
		std::array<ResourceBinding, MAX_RES_BINDING> ResBindings = {};
		int currentResBinding = 0;
		int vertexCurrentBinding = 0;

		void Reset()
		{
			currentResBinding = 0;
			vertexCurrentBinding = 0;
			for (auto& bind : ResBindings)
			{
				bind.currentHandle = 0;
				bind.currentHandle2 = 0;
			}
		}
	};

	enum class ItemType
	{
		RENDER,
		COMPUTE,
	};

	void SetDepthWrite(bool write);

	// render items learns about the view at submit
	struct Item
	{
		ItemType Type = ItemType::RENDER;
		ProgramHandle Program;
		PassID View;
		// buffers binded for draw call
		static constexpr int MAX_VERTEX_BINDING = 10;
		std::array<VertexBinding, MAX_VERTEX_BINDING> VertexBindings = {};
		IndexBufferHandle IndexBuffer;
		RenderState State;
		ResourceBindings Bindings;
		glm::vec3 GroupCount;
		glm::vec4 ClipRect; // x y w h
		RenderMode renderMode = RenderMode::WORLD;
		size_t MatrixIndex = 0;
		size_t ObjectCount = 1;
		size_t internalOffset = 0;

		void Reset()
		{
			MatrixIndex = 0;
			ObjectCount = 1;
			internalOffset = 0;
			Bindings.Reset();
		}

	};
	struct View
	{
		glm::vec4 Rect;
		glm::mat4 View;
		glm::mat4 Proj;
		FrameBufferHandle Fbh;
		int FreeIndex = 0;
		std::array<int,256> ItemsIndex;
	};

	struct Frame
	{
		int  CurrentRenderItemIndex = 0;
		Item m_renderItem[256];

		Item* CurrentRenderItem =&m_renderItem[CurrentRenderItemIndex];
		int LastView = 0;
		View Views[256];
		size_t FrameNumber = 0;
		size_t CurrentFreeMatrix = 0;
		std::array<glm::mat4, MAX_OBJECTS> Matricies;
		float deltaTime;
		void NextItem()
		{
			CurrentRenderItemIndex++;
			CurrentRenderItem = &m_renderItem[CurrentRenderItemIndex];
		}
		void Reset() 
		{
			LastView = 0;
			CurrentRenderItemIndex = 0;
			CurrentFreeMatrix = 0;
			CurrentRenderItem = &m_renderItem[CurrentRenderItemIndex];

		};
		// command to execute before Render/Compute API calls
		ResourceCommandBuffer CmdPre;
		// command to execute after Render/Compute API calls
		ResourceCommandBuffer CmdPost;
	};
	

	struct Model
	{
		TextureHandle Albedo;
		VertexLayoutHandle m_Layout;
		VertexBufferHandle m_VertexBuffer;
		IndexBufferHandle m_IndexBuffer;
	};

	SPtr<Model> LoadModel(std::string_view path);


	struct Submission
	{

		ResourceCommandBuffer& GetCommandBuffer(ResourceCommand command)
		{
			auto& cmdBuf = command < ResourceCommand::End ? Submit->CmdPre : Submit->CmdPost;

			cmdBuf.WriteByte(static_cast<uint8_t>(command));

			return cmdBuf;
		}

		Map<VertexBufferHandle, VertexLayoutHandle> VertexLayoutMap;
		Map<VertexLayoutHandle, VertexLayout> Layouts;
		// am not sure how we treat it in multithreading
		SPtr<Window> Window;
		Frame  Frames[THREADING+1];
		// the one registering user commands
		Frame* Submit;
		// the one doing API calls
		Frame* Render;
		
	};

	
	VertexLayout GetVertexLayout(VertexLayoutHandle handle);
	AttachmentHandle GetAttachmentHandle();
	TextureHandle GetTextureHandle();
	FrameBufferHandle GetFrameBufferHandle();

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment);

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry);

	ProgramHandle LoadComputeProgram(std::string_view cmp);

	ShaderHandle LoadShader(std::string_view shader);

	TextureHandle LoadTexture(std::string_view texture);
	TextureHandle LoadTextureFrom(Memory mem, int w, int h);

	TextureHandle CreateEmptyTexture(int width, int height, TextureFormat format, ResourceUsage usage,int mipLevels = 1, SampleCount e = SampleCount::e1,FilterMode min = FilterMode::Nearest, FilterMode mag = FilterMode::Nearest,int layers = 1, bool cube = false);

	void UpdateTexture(TextureHandle handle, uint8_t* data, size_t size);
	// read vulkan buffer on cpu
	size_t ReadTexture(TextureHandle handle, void* data);

	// use that on gpu cpu visible buffers
	void* ReadMappedPtr(ResourceType type, Handle<void>::Type handle);
	

	AttachmentHandle CreateAttachment(AttachmentType type, TextureFormat format, int width, int height, SampleCount samples, AttachmentHint hints);

	FrameBufferHandle CreateFramebuffer(const std::vector<AttachmentHandle>& attachments);
	
	TextureHandle GetColorTexture(FrameBufferHandle fb);
	void BindAttachmentAsTexture(std::string_view name, TextureHandle tex);
	
	void BindBuffer(std::string_view uniformName, 
		BufferHandle handle);
	
	// it is for sampled images
	void BindTexture(std::string_view uniform, TextureHandle handle);
	// it is for storage images
	void BindImage(std::string_view uniformName, TextureHandle handle);
	void BindTextures(std::string_view uniformName, const std::vector<TextureHandle>& handle);
	void BindImages(std::string_view uniformName, const std::vector<TextureHandle>& handle);
	std::vector<TextureHandle> GenerateMipMapsAsTextures(TextureHandle handle, int mipLevel);

	void SetWindow(SPtr<Window> window);
	void SubmitInit(InitParams);

	void SetTransform(const glm::mat4& world);
	void SetClipRect(int x,int y,int w, int h);
	void SetRenderMode(RenderMode mode);
	void SetViewTransform(PassID id, glm::mat4& view, glm::mat4& proj);
	void SetViewRect(PassID id , size_t x, size_t y, size_t width, size_t height);
	void SetFramebuffer(PassID id, FrameBufferHandle handle);

	void Submit(PassID id, ProgramHandle program,
		size_t instances = 1);

	void SubmitModel(SPtr<Model> model, PassID pass, ProgramHandle program, const glm::mat4& world);
	
	void SubmitCompute(PassID id, ProgramHandle program, size_t x, size_t y,size_t z);
	
	void BindIndexBuffer(IndexBufferHandle handle);
	// shader location
	void BindVertexBuffer(uint16_t location , VertexBufferHandle handle, VertexStreamMode mode = VertexStreamMode::VERTEX);
	VertexBufferHandle CreateVertexBuffer(Memory mem, VertexLayout& layout, ResourceUsage usage = ResourceUsage::Vertex);
	IndexBufferHandle CreateIndexBuffer(Memory mem);
	BufferHandle CreateBuffer (Memory mem , ResourceUsage usage);

	size_t GetCurrentFrame();
	void ExecuteFrame(float deltaTime,bool wait = false);

	void RunRender_(std::atomic_bool& isRunning);


	void UpdateImageRegionWithImage(
		const Memory& loadedImage,
		size_t width,
		size_t height,
		TextureHandle imageToUpdate,
		glm::vec3 offset,
		int layer);

	void SetData(BufferHandle, void*,size_t);

	void FillImage(TextureHandle handle, const glm::vec4& pixel,
		BufferHandle buffer, size_t bufferOffset);

	size_t GetSize(TextureHandle handle);
}

