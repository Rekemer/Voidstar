
include "Dependencies.lua"

workspace "Voidstar"
	architecture "x86_64"
	startproject "Sandbox"
    language "C++"
    cppdialect "C++17"
	configurations
	{
		"Debug",
		"Release",
		"Dist",
	}

	flags
	{
		"MultiProcessorCompile"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	VULKAN_HEADER = os.getenv("VULKAN_SDK") .. "/Include"
	GLM_HEADERS = "%{wks.solution}/Dependencies/glm/glm"
	TRACY_HEADERS =  "%{wks.solution}/Dependencies/Tracy/public"
	
	include "Voidstar"
	
	group "Dependencies"
		include "Dependencies/ImGui"
	group ""
	include "Sandbox"
	

	