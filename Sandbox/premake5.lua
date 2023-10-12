project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)
    files
    {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h"
    }
    includedirs
    {
        "src",
        "../"..GLM_HEADERS,
        "%{wks.solution}/Dependencies/ImGui",
        "../Voidstar/src",
        VULKAN_HEADER,
        "../" .. TRACY_HEADERS,
        "../" .. FREE_TYPE_HEADERS,
        "../" .. LOG_HEADERS
    }

    links { "Voidstar","Freetype"  }
    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        optimize "On"