include (_SCRIPT_DIR .. "/Dependencies.lua")

outdir = outdir or "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

function LinkLumora()
    includedirs { LumoraIncludeDirs }
    defines     { "flecs_STATIC" }
    links       { "Lumora", "%{Library.VULKAN}", "%{Library.SHADERC}" }
    libdirs     { VULKAN_LIB_DIR }
end

-- ---------------------------------------------------------------------
-- Engine + dependency projects.
-- ---------------------------------------------------------------------
group "Dependencies"
    include (_SCRIPT_DIR .. "/External/glad")
    include (_SCRIPT_DIR .. "/External/glfw")
    include (_SCRIPT_DIR .. "/External/imgui")
    include (_SCRIPT_DIR .. "/External/flecs")
    include (_SCRIPT_DIR .. "/External/lua")
group ""

group "Engine"
    include (_SCRIPT_DIR .. "/Lumora")
group ""
