project "LumoraApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir ("%{wks.location}/bin/" .. outdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/bin-int/" .. outdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        -- Engine
        "%{wks.location}/Lumora/src",
        -- DEPS
        "%{IncludeDir.GLFW}",
        -- BGFX
        "%{IncludeDir.BGFX}",
        "%{IncludeDir.BX}",
        "%{IncludeDir.BIMG}",
        -- VULKAN
        "%{IncludeDir.VULKAN}",
        -- SPDLOG
        "%{IncludeDir.SPDLOG}",
        -- LUA
        "%{IncludeDir.LUA}",
        "%{IncludeDir.SOL}",

        "%{IncludeDir.VISITSTRUCT}",
        "%{IncludeDir.FILEWATCH}",

        "%{IncludeDir.IMGUI}",
        "%{IncludeDir.GLM}"
    }

    libdirs
    {
        VULKAN_LIB_DIR
    }

    links
    {
        "Lumora",

        "%{Library.VULKAN}",
        "glfw",
        "bgfx", "bimg", "bx", -- BGFX
        "lua",
        "ImGui"
    }

    -- If VS
    filter "action:vs*"
        -- Enable /utf-8 flag for Visual Studio
        buildoptions { "/utf-8" }

    filter "system:windows"
        systemversion "latest"

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

    setBxCompat()

    -- Clear the filters
    filter {}

    -- Post build commands
    postbuildcommands
    {
        '{COPYFILE} "%{cfg.buildtarget.relpath}" "%{wks.location}/bin/"'
    }
