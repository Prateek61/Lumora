project "bimg"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    exceptionhandling "Off"
    rtti "Off"

    targetdir ("bin/" .. outdir .. "/%{prj.name}")
    objdir ("bin/int/" .. outdir .. "/%{prj.name}")

    files
    {
        "bimg/include/bimg/*.h",
        "bimg/src/image.cpp",
        "bimg/src/image_gnf.cpp",
        "bimg/src/*.h",
        "bimg/3rdparty/astc-encoder/source/*.cpp"
    }

    includedirs
    {
        "%{IncludeDir.BX}",
        "bimg/include",
        "bimg/3rdparty/astc-encoder",
        "bimg/3rdparty/astc-encoder/include"
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