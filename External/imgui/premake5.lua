project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir ("bin/" .. outdir .. "/%{prj.name}")
    objdir ("bin/int/" .. outdir .. "/%{prj.name}")

    files
    {
        "imgui/*.h",
        "imgui/*.cpp",

        "imgui/backends/imgui_impl_opengl3.h",
        "imgui/backends/imgui_impl_opengl3.cpp",

        "imgui/backends/imgui_impl_glfw.h",
        "imgui/backends/imgui_impl_glfw.cpp",

        "imgui/backends/imgui_impl_vulkan.h",
        "imgui/backends/imgui_impl_vulkan.cpp"
    }

    includedirs
    {
        "imgui/",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.VULKAN}",
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        optimize "Debug"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"
        defines { "NDEBUG" }

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"
        defines { "NDEBUG" }

    -- Clear the filters
    filter {}
