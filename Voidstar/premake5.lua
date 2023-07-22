project "Voidstar"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    targetdir ("%{wks.location}/bin/%{prj.name}/"..outputdir)
    objdir ("%{wks.location}/bin-int/%{prj.name}/"..outputdir)

    files
    {
        "%{prj.location}/src/**.cpp",
        "%{prj.location}/src/**.h",
        "%{wks.location}/Dependencies/Tracy/public/TracyClient.cpp",
    }

    pchheader "Prereq.h"
	pchsource "src/Prereq.cpp"
    includedirs
    {
        os.getenv("VULKAN_SDK") .. "/Include",
        "%{wks.location}/Dependencies/spdlog/include",
        "%{wks.location}/Dependencies/GLFW/include",
        "%{wks.location}/Dependencies/glm/glm",
        "%{wks.location}/Dependencies/spdlog/include/spdlog",
        "%{wks.location}/Dependencies/",
        "%{wks.location}/Dependencies/Tracy/public",
        "%{wks.location}/Dependencies/ImGui",
        "%{prj.location}/src"
        
    }
    libdirs { "%{wks.location}/Dependencies/GLFW/lib-vc2019", os.getenv("VULKAN_SDK") .. "/Lib" }
    
    links { "glfw3.lib", "vulkan-1.lib", "ImGui"}

    -- Exclude tracy.cpp from using precompiled headers
    filter "files:../Dependencies/Tracy/public/**.cpp"
    flags { "NoPCH" }
    
    filter "configurations:Debug"
        defines "VS_DEBUG"
        symbols "On"
        
    filter "configurations:Release"
        defines "VS_RELEASE"
        defines "TRACY_ENABLE"
        optimize "On"
