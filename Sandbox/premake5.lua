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
        "%{wks.solution}/Dependencies/glm/glm",
        "%{wks.solution}/Dependencies/ImGui/imgui",
        "../Voidstar/src",
    }

    links { "Voidstar",  }
    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        optimize "On"