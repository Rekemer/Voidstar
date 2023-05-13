workspace "Voidstar"
    architecture "x64"
    configurations
    {
        "Debug",
        "Release"
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
project "Voidstar"
    location "Voidstar"
    kind "StaticLib"
    language "C++"
    targetdir ("bin/%{prj.name}"..outputdir)
    objdir ("bin-int/%{prj.name}"..outputdir)
    files
    {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }
    includedirs
    {
        os.getenv("VULKAN_SDK") .. "/Include",
        solution().basedir .. "/Dependencies/spdlog/include",
        solution().basedir .. "/Dependencies/GLFW/include",
        solution().basedir .. "/Dependencies/glm/glm"
    }
    libdirs {   solution().basedir .. "/Dependencies/GLFW/lib-vc2019", os.getenv("VULKAN_SDK") .. "/Lib" }

    links { "glfw3.lib", "vulkan-1.lib" }
    defines
    {
        "VS_RELEASE",
        "VS_DEBUG"
    }
    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        optimize "On"
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    targetdir ("bin/%{prj.name}"..outputdir)
    objdir ("bin-int/%{prj.name}"..outputdir)
    files
    {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h"
    }
    includedirs
    {
        "Voidstar/src",
        solution().basedir .. "/Dependencies/glm/glm"
    }

    links { "Voidstar" }

    defines
    {
        "VS_RELEASE",
        "VS_DEBUG"
    }
    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        optimize "On"