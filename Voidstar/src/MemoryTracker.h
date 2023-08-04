#pragma once
#include <cstdlib>

namespace Voidstar
{
	struct AllocationMetrics
	{
		size_t allocated = 0;
		size_t freed = 0;
		size_t alloc{ 0 };
		size_t dealloc{ 0 };
		size_t Current() { return allocated - freed; }
		size_t Leaks() { return alloc - dealloc; }
	};
	inline AllocationMetrics allocationMetrics;
	
}
 void* operator new(size_t size);


 void operator delete(void* ptr);
