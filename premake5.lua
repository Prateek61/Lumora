-- =====================================================================
-- Lumora — standalone workspace
--
-- This builds the engine plus LumoraApp (the engine's own dev harness:
-- Glimmer / Sandbox demos). Consumer projects do NOT use this file —
-- they declare their own workspace and `include "Lumora/LumoraProjects.lua"`.
-- =====================================================================

workspace "Lumora"
    architecture "x64"
    startproject "LumoraApp"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

-- Engine + all dependency projects (also defines `outdir` and LinkLumora()).
include "LumoraProjects.lua"

-- The engine's own test application.
group "App"
    include "LumoraApp"
group ""
