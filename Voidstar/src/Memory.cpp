#include "Prereq.h"
#include "Memory.h"
#include "stb_image.h"
namespace Voidstar
{
	void FreeMemory(Memory& mem)
	{
        switch (mem.allocate)
        {
            case AllocateWay::NEW:    delete[] mem.data; break;
            case AllocateWay::STBI:   stbi_image_free(mem.data); break;
            default: break; 
        }
        mem.data = nullptr;
	}
}