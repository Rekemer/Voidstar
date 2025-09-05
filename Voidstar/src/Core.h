#pragma once
//#ifdef VOIDSTAR_BUILD
//#define VOIDSTAR_API    // when building the DLL
//#else
//#define VOIDSTAR_API    // when consuming the DLL
//#endif
#define VOIDSTAR_API    // when building the DLL


#define VOID_HANDLE(_name)\
	struct _name { uint16_t idx; };