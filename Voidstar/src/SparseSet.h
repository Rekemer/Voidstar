#pragma once
#include <cstdint>
#include <array>
namespace Voidstar
{
	constexpr uint16_t ENTRIES_AMOUNT = uint16_t(-1);
	
	class SparseSet
	{
	public:
		SparseSet();
		uint16_t GetId();

	private:
		size_t entriesAmount = 0;
		// has actual ids
		std::array<uint16_t, ENTRIES_AMOUNT> dense;
		// has indexes[] of ids in dense
		std::array<uint16_t, ENTRIES_AMOUNT> sparse;
	};
}