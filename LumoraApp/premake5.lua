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
        "src"
    }

    -- Engine: include dirs, links, libdirs, and the flecs_STATIC define.
    LinkLumora()

    -- If VS
    filter "action:vs*"
        buildoptions { "/utf-8" }

    filter "system:windows"
        systemversion "latest"

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

    -- Post build commands
    -- shaderc_shared.dll rides next to the exe: the Vulkan backend compiles GLSL at runtime through it.
    -- Copied to both the build target dir (VS runs from there) and bin/ (build_run.py runs from there).
    postbuildcommands
    {
        '{COPYFILE} "%{cfg.buildtarget.relpath}" "%{wks.location}/bin/"',
        '{COPYFILE} "' .. VULKAN_BIN_DIR .. '/shaderc_shared.dll" "%{cfg.buildtarget.directory}"',
        '{COPYFILE} "' .. VULKAN_BIN_DIR .. '/shaderc_shared.dll" "%{wks.location}/bin/"'
    }
