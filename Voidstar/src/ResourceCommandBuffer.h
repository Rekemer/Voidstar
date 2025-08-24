#pragma once
#include <vector>
#include <string_view>
namespace Voidstar
{
	enum class ResourceCommand : uint8_t
	{
		RendererInit,
		RendererShutdownBegin,
		CreateVertexLayout,
		CreateIndexBuffer,
		CreateVertexBuffer,
		CreateDynamicIndexBuffer,
		UpdateDynamicIndexBuffer,
		CreateDynamicVertexBuffer,
		UpdateDynamicVertexBuffer,
		CreateShader,
		// link shaders
		CreateProgram,
		CreateTexture,
		UpdateTexture,
		ResizeTexture,
		CreateFrameBuffer,
		CreateUniform,
		UpdateViewName, // ?
		InvalidateOcclusionQuery,
		SetName,
		End,
		RendererShutdownEnd,
		DestroyVertexLayout,
		DestroyIndexBuffer,
		DestroyVertexBuffer,
		DestroyDynamicIndexBuffer,
		DestroyDynamicVertexBuffer,
		DestroyShader,
		DestroyProgram,
		DestroyTexture,
		DestroyFrameBuffer,
		DestroyUniform,
		ReadTexture,
	};

	class ResourceCommandBuffer
	{

	public:
		using Iterator = std::vector<uint8_t>::iterator;

		void WriteByte(uint8_t command);
		
		void Write(std::string_view str);
		
		std::string_view ReadString();

		uint8_t ReadByte();
	private:

		template<typename T>
		T Read()
		{
			T val;
			std::memcpy(&val, &(*current), sizeof(T));
			current += sizeof(T);
			return val;
		}
		// iterators can get invalidated
		size_t current;
		std::vector<uint8_t> commands;
	};

}