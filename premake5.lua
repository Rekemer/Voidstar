
include "Dependencies.lua"

workspace "Voidstar"
	architecture "x86_64"
	startproject "Sandbox"
    language "C++"
    cppdialect "C++17"
	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


	
	include "Voidstar"
	
	group "Dependencies"
		include "Dependencies/ImGui"
	group ""
	include "Sandbox"
	

	