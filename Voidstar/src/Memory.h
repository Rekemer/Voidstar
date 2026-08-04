#pragma once
#include <cstdint>
namespace Voidstar
{
	enum class AllocateWay
	{
		NONE = 0,
		NEW,
		STBI,
	};


	struct Memory
	{
		uint8_t* data = nullptr; 
		size_t size;
		// no point to clean up 
		// after index or vertex buffer, it is usually owned by a vector
		
		// but with textures ww might want to clean up since
		// stbi_image_free should be called after resource command is proccessed
		AllocateWay allocate;
		bool cleanUp = false;
	};

	// not sure if we need to use it after each vulkan allocation 
	// from cpu memory, 
	void FreeMemory(Memory& mem);


}