project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir ("bin/" .. outdir .. "/%{prj.name}")
    objdir ("bin/int/" .. outdir .. "/%{prj.name}")

    files
    {
        "imgui/*.h",
        "imgui/*.cpp"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        optimize "Debug"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"

    -- Clear the filters
    filter {} 
    