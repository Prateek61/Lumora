project "bx"
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
        "bx/include/bx/*.h",
        "bx/include/bx/inline/*.inl",
        "bx/src/*.cpp"
    }
    excludes
    {
        "bx/src/amalgamated.**",
        "bx/src/crtnone.cpp"
    }

    includedirs
    {
        "bx/include",
        "bx/3rdparty"
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

    filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"

    -- Clear filters
    filter {}

    setBxCompat()