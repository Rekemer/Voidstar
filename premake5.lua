workspace "Voidstar"
    architecture "x64"
    language "C++"
    cppdialect "C++17"
    configurations
    {
        "Debug",
        "Release"
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    targetdir ("bin/%{prj.name}/"..outputdir)
    objdir ("bin-int/%{prj.name}/"..outputdir)
    files
    {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h"
    }
    includedirs
    {
        "Voidstar/src",
        solution().basedir .. "/Dependencies/glm/glm"
    }

    links { "Voidstar-Core" }
    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        optimize "On"

project "Voidstar-Core"
    location "Voidstar"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    targetdir ("bin/%{prj.name}/"..outputdir)
    objdir ("bin-int/%{prj.name}/"..outputdir)

    files
    {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h",
        "%{prj.location}/Dependencies/Tracy/public/TracyClient.cpp",
    }

    pchheader "Prereq.h"
	pchsource "Voidstar/src/Prereq.cpp"
    includedirs
    {
        os.getenv("VULKAN_SDK") .. "/Include",
        "%{prj.location}/Dependencies/spdlog/include",
        "%{prj.location}/Dependencies/GLFW/include",
        "%{prj.location}/Dependencies/glm/glm",
        "%{prj.location}/Dependencies/spdlog/include/spdlog",
        "%{prj.location}/Dependencies/",
        "%{prj.location}/Dependencies/Tracy/public",
        
    }
    libdirs { "%{prj.location}/Dependencies/GLFW/lib-vc2019", os.getenv("VULKAN_SDK") .. "/Lib" }
    
    links { "glfw3.lib", "vulkan-1.lib"}

    -- Exclude tracy.cpp from using precompiled headers
    filter "files:Voidstar/Dependencies/Tracy/public/**.cpp"
    flags { "NoPCH" }
    
    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        defines "TRACY_ENABLE"
        optimize "On"
