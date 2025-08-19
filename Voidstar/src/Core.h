#pragma once
#ifdef VOIDSTAR_BUILD
#define VOIDSTAR_API __declspec(dllexport)   // when building the DLL
#else
#define VOIDSTAR_API __declspec(dllimport)   // when consuming the DLL
#endif