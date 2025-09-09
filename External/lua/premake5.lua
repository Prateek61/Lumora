project "Lua"
    kind "StaticLib"
    language "C"
    
    targetdir ("bin/" .. outdir .. "/%{prj.name}")
    objdir ("bin/int/" .. outdir .. "/%{prj.name}")

    files
    {
        "lua/*.h",
        "lua/onelua.c"
    }

    includedirs
    {
        "lua"
    }

    defines
    {
        "MAKE_LIB"
    }

    filter "configurations:Debug"
        defines{"_DEBUG", "LUA_USE_ASSERT"}
        optimize "Debug"
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines{"NDEBUG", "LUA_USE_ASSERT"}
        optimize "On"
        symbols "On"
        runtime "Release"

    filter "configurations:Dist"
        defines{"NDEBUG"}
        optimize "Full"
        runtime "Release"

    -- Clear the filters
    filter {}
