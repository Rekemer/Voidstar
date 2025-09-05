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

	// no alighment consideration
	class ResourceCommandBuffer
	{

	public:
		using Iterator = std::vector<uint8_t>::iterator;


		template<class T>
		void WriteObject(const T& v) {

			static_assert(std::is_trivially_copyable_v<T>, "POD only");
			size_t off = commands.size();
			commands.resize(off + sizeof(T));
			std::memcpy(commands.data() + off, &v, sizeof(T));
		}

		template<class T>
		T ReadObject()
		{
			static_assert(std::is_trivially_copyable_v<T>, "POD only");
			T obj;
			if (sizeof(T) + current > commands.size())
				throw std::out_of_range("ResourceCommandBuffer Reading out of bounds");

			std::memcpy(&obj, commands.data() + current, sizeof(T));
			current += sizeof(T);
			return obj;
		}
		bool IsReadable();
		void Reset();
		void WriteByte(uint8_t command);
		void WriteString(std::string_view str);
		std::string_view ReadString();
		uint8_t ReadByte();
		template<typename T>
		T Read()
		{
			T val;
			std::memcpy(&val, commands.data() + current, sizeof(T));
			current += sizeof(T);
			return val;
		}
	private:

		// iterators can get invalidated
		size_t current;
		std::vector<uint8_t> commands;
	};

}