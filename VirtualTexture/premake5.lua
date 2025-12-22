project "VirtualTexture"
    kind "ConsoleApp"
    language "C++"
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
        "../Dependencies/ImGui",
        "../Voidstar/src",
        VULKAN_HEADER,
        "../" .. TRACY_HEADERS,
        "../" .. FREE_TYPE_HEADERS,
        "../" .. SPDLOG_HEADERS
    }
    libdirs { os.getenv("VULKAN_SDK") .. "/Lib" }
    links { "Voidstar","Freetype", "ImGui","vulkan-1.lib"  }

-- if we are to make renderer a dll 
--     postbuildcommands {
--        ('{COPY} "%{wks.location}/bin/Voidstar/' .. outputdir .. '/Voidstar.dll" "%{cfg.targetdir}"')
--    }

    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        optimize "On"

