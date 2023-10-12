
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
	FREE_TYPE_HEADERS = "%{wks.solution}/Dependencies/freetype/include"
	SPDLOG_HEADERS = "%{wks.location}/Dependencies/spdlog/include"
	include "Voidstar"
	
	group "Dependencies"
		include "Dependencies/ImGui"
		include "Dependencies/freetype"
	group ""
	include "Sandbox"
	

	