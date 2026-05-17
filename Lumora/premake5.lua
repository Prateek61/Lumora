project "Lumora"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/bin-int/" .. outdir .. "/%{prj.name}")

    pchheader "LMPCH.h"
    pchsource "src/LMPCH.cpp"

    defines
    {
        "flecs_STATIC"
    }

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
        -- VULKAN
        "%{IncludeDir.VULKAN}",
        -- OPENGL
        "%{IncludeDir.GLAD}",

        "%{IncludeDir.VISITSTRUCT}",
        "%{IncludeDir.FILEWATCH}",

        "%{IncludeDir.IMGUI}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.FLECS}"
    }

    libdirs
    {
        VULKAN_LIB_DIR
    }

    links
    {
        "%{Library.VULKAN}",
        "glfw",
        "Lua",
        "glad",
        "ImGui",
        "flecs"
    }

    filter "system:windows"
        links
        {
            "%{Library.Win.GDI32}",
            "%{Library.Win.KERNEL32}",
            "%{Library.Win.PSAPI}"
        }

    filter "system:linux"
        links
        {
            "%{Library.Linux.DL}",
            "%{Library.Linux.GL}",
            "%{Library.Linux.X11}",
            "%{Library.Linux.PTHREAD}"
        }

    filter "system:macosx"
        links
        {
            "%{Library.MacOS.QUARTZ}",
            "%{Library.MacOS.COCOA}",
            "%{Library.MacOS.IOKIT}",
            "%{Library.MacOS.METAL}",
            "%{Library.MacOS.COREVIDEO}"
        }

    filter {}

    -- If VS
    filter "action:vs*"
        -- Enable /utf-8 flag for Visual Studio
        buildoptions { "/utf-8" }


    filter "configurations:Debug"
        defines
        {
            "LM_DEBUG",
            "_DEBUG",
        }
        runtime "Debug"
        optimize "Debug"
        symbols "On"

    filter "configurations:Release"
        defines
        {
            "LM_RELEASE",
            "NDEBUG",
        }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines
        {
            "LM_DIST",
            "NDEBUG",
        }
        runtime "Release"
        optimize "Full"

    -- Clear the filters
    filter {}