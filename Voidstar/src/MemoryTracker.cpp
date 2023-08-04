#include "Prereq.h"
#include"MemoryTracker.h"
#include"tracy/Tracy.hpp"
void* operator new(size_t size)
{

	//Voidstar::allocationMetrics.allocated += size;
	Voidstar::allocationMetrics.alloc += 1;
	auto ptr = malloc(size);
	TracyAllocS(ptr, size,5);
	return ptr;

}

void operator delete(void* ptr)
{
	//Voidstar::allocationMetrics.freed += size;
	Voidstar::allocationMetrics.dealloc += 1;
	TracyFreeS(ptr,5);
	free(ptr);
}