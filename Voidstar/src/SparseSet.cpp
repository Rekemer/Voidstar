#include "Prereq.h"
#include "SparseSet.h"
// for std:iota
#include <numeric>

namespace Voidstar 
{

	SparseSet::SparseSet()
	{
		// to fill like 0,1,2,3,4,5...
		std::iota(dense.begin(), dense.end(),0);
		std::fill(sparse.begin(), sparse.end(), 0);
	}
	uint16_t SparseSet::GetId()
	{
		auto index = entriesAmount++;
		auto handle = dense[index];
		sparse[handle] = index;
		return handle;
	}
}