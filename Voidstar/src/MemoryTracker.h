#pragma once
#include <cstdlib>
namespace Voidstar
{
	struct AllocationMetrics
	{
		size_t allocated = 0;
		size_t freed = 0;
		size_t Current() { return allocated - freed; }
	};
	inline AllocationMetrics allocationMetrics;
	
}
inline void* operator new(size_t size)
{
	Voidstar::allocationMetrics.allocated += size;
	return malloc(size);
}

inline void operator delete(void* ptr, size_t size)
{
	Voidstar::allocationMetrics.freed += size;

	return free(ptr);
}