include "Dependencies.lua"

workspace "Lumora"
    architecture "x64"
    startproject "LumoraApp"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    outdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "App"
    include "LumoraApp"
group ""

group "Core"
    include "Lumora"
group ""

group "Dependencies"
    include "External/glad"

    include "External/glfw"

    include "External/imgui"
    include "External/flecs"

    include "External/lua"
group ""