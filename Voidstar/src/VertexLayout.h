#pragma once
#include<vector>
#include<cassert>
namespace Voidstar
{
	enum class ShaderDataType
	{
		FLOAT4,
		FLOAT3,
		FLOAT2,
		FLOAT,

	};
	struct VertexLayout
	{
		struct VertexLayoutElement
		{
			ShaderDataType type;
			size_t offset;
		};

		VertexLayout& Add(ShaderDataType type)
		{
			m_Elements.push_back(VertexLayoutElement{ type,m_CurrentOffset });
			m_CurrentOffset += GetSize(type);
			return *this;
		}
	private:
		int GetSize(ShaderDataType type)
		{
			switch (type)
			{
			case Voidstar::ShaderDataType::FLOAT4:
				return sizeof(float) * 4;
				break;
			case Voidstar::ShaderDataType::FLOAT3:
				return sizeof(float) * 3;
				break;
			case Voidstar::ShaderDataType::FLOAT2:
				return sizeof(float) * 2;
				break;
			case Voidstar::ShaderDataType::FLOAT:
				return sizeof(float);
				break;
			default:
				assert(false);
				return -1;
				break;
			}
		}
		friend class Renderer;
		std::vector<VertexLayoutElement> m_Elements;
		size_t m_CurrentOffset = 0;
	};
}
