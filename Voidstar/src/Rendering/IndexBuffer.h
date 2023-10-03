#pragma once
#include"Buffer.h"
namespace Voidstar
{
	typedef uint32_t IndexType;
	class IndexBuffer : public Buffer
	{
	public:
		IndexBuffer(BufferInputChunk& input,size_t indexAmount, vk::IndexType indexType) :
			Buffer(input),
			m_IndexAmount{ indexAmount },
			m_IndexType{ indexType }

		{

		}
		size_t GetIndexAmount() { return m_IndexAmount; }
		vk::IndexType  GetIndexType() { return m_IndexType; }
	private:
		size_t m_IndexAmount;
		vk::IndexType m_IndexType;
	};
}