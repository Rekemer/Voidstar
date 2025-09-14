#pragma once
#include <cstdint>
#include <array>
// for std:iota
#include <numeric>
namespace Voidstar
{
	constexpr uint16_t ENTRIES_AMOUNT = uint16_t(-1);
	
	template<typename T>
	class SparseSet
	{
	public:
		SparseSet()
		{
			// to fill like 0,1,2,3,4,5...
			std::iota(dense.begin(), dense.end(), 0);
			std::fill(sparse.begin(), sparse.end(), 0);
		}
		
		T GetId()
		{
			auto index = entriesAmount++;
			auto handle = dense[index];
			sparse[handle] = index;
			return T{ handle };
		}

	private:
		size_t entriesAmount = 0;
		// has actual ids
		std::array<uint16_t, ENTRIES_AMOUNT> dense;
		// has indexes[] of ids in dense
		std::array<uint16_t, ENTRIES_AMOUNT> sparse;
	};
}