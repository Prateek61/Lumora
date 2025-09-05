project "bgfx"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    exceptionhandling "Off"
    rtti "Off"

    defines "__STDC_FORMAT_MACROS"

    targetdir ("bin/" .. outdir .. "/%{prj.name}")
    objdir ("bin/int/" .. outdir .. "/%{prj.name}")

    files
    {
        "bgfx/include/bgfx/**.h",
        "bgfx/src/*.cpp",
        "bgfx/src/*.h",
    }
    excludes
    {
        "bgfx/src/amalgamated.cpp"
    }

    includedirs
    {
        "%{IncludeDir.BX}",
        "%{IncludeDir.BIMG}",
        "bgfx/include",
        "bgfx/3rdparty",
        "bgfx/3rdparty/khronos",
        "bgfx/3rdparty/directx-headers/include/directx",
    }

    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
        excludes
        {
            "bgfx/src/glcontext_glx.cpp",
            "bgfx/src/glcontext_egl.cpp",
        }

    filter "system:macosx"
        files
        {
            "src/*.mm"
        }

    filter "configurations:Debug"
        defines
        {
            "_DEBUG",
            "BX_CONFIG_DEBUG=1"
        }
        optimize "Debug"
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines
        {
            "NDEBUG",
            "BX_CONFIG_DEBUG=0"
        }
        optimize "On"
        symbols "On"
        runtime "Release"

    filter "configurations:Dist"
        defines
        {
            "NDEBUG",
            "BX_CONFIG_DEBUG=0"
        }
        optimize "Full"
        runtime "Release"

    -- Clear the filters 
    filter {}

    setBxCompat()
