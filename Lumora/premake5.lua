project "Lumora"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/bin-int/" .. outdir .. "/%{prj.name}")

    pchheader "LMPCH.h"
    pchsource "src/LMPCH.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src/",
       

        "%{IncludeDir.GLFW}",
        -- Logger
        "%{IncludeDir.SPDLOG}",
        -- Lua
        "%{IncludeDir.SOL}",
        "%{IncludeDir.LUA}",
        -- BGFX
        "%{IncludeDir.BGFX}",
        "%{IncludeDir.BX}",
        "%{IncludeDir.BIMG}",

        "%{IncludeDir.VISITSTRUCT}",
        "%{IncludeDir.FILEWATCH}",

        "%{IncludeDir.IMGUI}"
    }

    links
    {
        "glfw",
        "Lua",
        "bgfx", "bimg", "bx", -- BGFX
        "ImGui"
    }

    -- If VS
    filter "action:vs*"
        -- Enable /utf-8 flag for Visual Studio
        buildoptions { "/utf-8" }


    filter "configurations:Debug"
        defines
        {
            "LM_DEBUG",
            "_DEBUG",
            "BX_CONFIG_DEBUG=1"
        }
        runtime "Debug"
        optimize "Debug"
        symbols "On"

    filter "configurations:Release"
        defines
        {
            "LM_RELEASE",
            "NDEBUG",
            "BX_CONFIG_DEBUG=0"
        }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines
        {
            "LM_DIST",
            "NDEBUG",
            "BX_CONFIG_DEBUG=0"
        }
        runtime "Release"
        optimize "Full"

    -- Clear the filters
    filter {}