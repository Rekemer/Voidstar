
include "Dependencies.lua"

workspace "Voidstar"
	architecture "x86_64"
	startproject "Sandbox"
    language "C++"
    cppdialect "C++23"
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
	GLM_HEADERS = "%{wks.location}/Dependencies/glm/glm"
	SPIRV_CROSS_HEADERS = "%{wks.location}/Dependencies/SPIRV-Cross"
	TRACY_HEADERS =  "%{wks.solution}/Dependencies/Tracy/public"
	FREE_TYPE_HEADERS = "%{wks.solution}/Dependencies/freetype/include"
	SPDLOG_HEADERS = "%{wks.location}/Dependencies/spdlog/include"
	CGLTF_HEADERS = "%{wks.location}/Dependencies/cgltf"
	include "Voidstar"
	
	group "Dependencies"
		include "Dependencies/ImGui"
		include "Dependencies/freetype"
		include "Dependencies/SPIRV-Cross"
	group ""
	include "VirtualTexture"
	include "Sandbox"
	

	