#pragma once
#include <array>
namespace Voidstar
{
	template<typename T, size_t Size>
	class FixedArray
	{
	public:
		void Add(std::vector<T> elements)
		{
			assert((Size - m_FreeIndex + 1) > elements.size());
			auto startIndex = m_FreeIndex;
			auto objCount = elements.size();
			size_t elementIndex = 0;
			for (auto i = startIndex;
				i < startIndex + objCount;
				i++)
			{
				m_Array[i] = elements[elementIndex++];
			}
			m_FreeIndex = startIndex + objCount;
		}
		void Reset()
		{
			m_FreeIndex = 0;
		}
		size_t GetFreeIndex() { return m_FreeIndex; }

		T* GetPtrAt(size_t index) 
		{
			return &m_Array.at(index);
		}
		T GetElementAt(size_t index)
		{
			return m_Array.at(index);
		}
		private:
			std::array<T, Size> m_Array;
			size_t m_FreeIndex = 0;

	};
}