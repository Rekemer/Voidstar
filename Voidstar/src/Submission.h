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

namespace Voidstar
{
	enum class ResourceType : uint8_t
	{
		UniformBuffer,
		StorageBuffer,
		SampledImage,
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

	using ProgramHandle = Handle<ProgramTag>;
	using ShaderHandle = Handle<ShaderTag>;
	using VertexBufferHandle = Handle<VertexBufferTag>;
	using IndexBufferHandle = Handle<IndexBufferTag>;
	using FrameBufferHandle = Handle<FramebufferTag>;
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

	enum class UpdateHint {
		Immutable,   // uploaded once, then never touched by CPU
		Static,      // updated occasionally (loading screen, level change)
		Dynamic,     // updated frequently (per frame or per few frames)
		Readback     // GPU → CPU reads (screenshots, queries)
	};

	struct Memory
	{
		uint8_t* data; //!< Pointer to data.
		size_t size; //!< Data size.
	};

	struct VertexBinding
	{
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
		bool        enabled = false;
		BlendFactor srcColor = BlendFactor::One;
		BlendFactor dstColor = BlendFactor::Zero;
		BlendOp     colorOp = BlendOp::Add;
		BlendFactor srcAlpha = BlendFactor::One;
		BlendFactor dstAlpha = BlendFactor::Zero;
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
		TextureHandle handle;
		bool dirty;
	};

	// render items learns about the view at submit
	struct RenderItem
	{
		ProgramHandle Program;
		PassID View;
		// buffers binded for draw call
		static constexpr int MAX_VERTEX_BINDING = 10;
		static constexpr int MAX_RES_BINDING = 10;
		std::array<VertexBinding, MAX_VERTEX_BINDING> Bindings = {};
		int currentBinding = 0;
		IndexBufferHandle IndexBuffer;
		RenderState State;
		std::array<ResourceBinding, MAX_RES_BINDING> ResBindings= {};
		int currentResBinding = 0;


		void Reset()
		{
			currentResBinding = 0;
			currentBinding = 0;
		}

	};
	struct View
	{
		glm::vec4 Rect;
		glm::mat4 View;
		glm::mat4 Proj;
		FrameBufferHandle Fbh;
	};

	struct Frame
	{
		int  CurrentRenderItemIndex = 0;
		RenderItem m_renderItem[256];
		RenderItem* CurrentRenderItem =&m_renderItem[CurrentRenderItemIndex];
		View Views[256];

		void NextItem()
		{
			CurrentRenderItemIndex++;
			CurrentRenderItem = &m_renderItem[CurrentRenderItemIndex];
		}
		void Reset() 
		{
			CurrentRenderItemIndex = 0;
			CurrentRenderItem = &m_renderItem[CurrentRenderItemIndex];
		};
		// command to execute before Render/Compute API calls
		ResourceCommandBuffer CmdPre;
		// command to execute after Render/Compute API calls
		ResourceCommandBuffer CmdPost;
	};
	

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
		Frame  Frames[1];
		// the one registering user commands
		Frame* Submit;
		// the one doing API calls
		Frame* Render;
		
	};

	
	VertexLayout GetVertexLayout(VertexLayoutHandle handle);

	

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment);

	ProgramHandle LoadProgram(std::string_view vertex, std::string_view fragment, std::string_view geometry);

	ProgramHandle LoadProgram(std::string_view program, ShaderType type);

	ShaderHandle LoadShader(std::string_view shader, ShaderType type);

	TextureHandle LoadTexture(std::string_view texture);

	UniformHandle CreateUniform(std::string_view name,
		ResourceType kind, size_t num = 1);


	void BindTexture(std::string_view uniform, TextureHandle handle);
	
	void SetWindow(SPtr<Window> window);
	void SubmitInit(InitParams);


	void SetViewTransform(PassID id, glm::mat4& view, glm::mat4& proj);
	void SetViewRect(PassID id , size_t x, size_t y, size_t width, size_t height);

	void Submit(PassID id, ProgramHandle program);
	void BindIndexBuffer(IndexBufferHandle handle);
	// shader location
	void BindVertexBuffer(uint16_t location , VertexBufferHandle handle);

	VertexBufferHandle CreateVertexBuffer(Memory mem, VertexLayout& layout);
	IndexBufferHandle CreateIndexBuffer(Memory mem);

	void ExecuteFrame(float deltaTime);

}

