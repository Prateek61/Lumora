local LumoraDir = _SCRIPT_DIR

-- Vulkan SDK (required by the renderer).
vk_sdk = os.getenv("VULKAN_SDK")
if (vk_sdk == nil) then
    print("VULKAN_SDK environment variable not set. Please install the Vulkan SDK and set the VULKAN_SDK environment variable.")
    os.exit(1)
end
VULKAN_LIB_DIR = vk_sdk .. "/Lib"
VULKAN_BIN_DIR = vk_sdk .. "/Bin"

IncludeDir = {}
-- The engine's own public headers — for consumers compiling against Lumora.
IncludeDir["LUMORA"]      = LumoraDir .. "/Lumora/src"
-- Vulkan
IncludeDir["VULKAN"]      = vk_sdk .. "/Include"
-- OpenGL loader
IncludeDir["GLAD"]        = LumoraDir .. "/External/glad/include"
-- Windowing
IncludeDir["GLFW"]        = LumoraDir .. "/External/glfw/glfw/include"
-- Lua
IncludeDir["LUA"]         = LumoraDir .. "/External/lua/lua/"
IncludeDir["SOL"]         = LumoraDir .. "/External/sol/include/"
-- Maths
IncludeDir["GLM"]         = LumoraDir .. "/External/glm/"
-- Logging
IncludeDir["SPDLOG"]      = LumoraDir .. "/External/spdlog/include"
-- ImGui
IncludeDir["IMGUI"]       = LumoraDir .. "/External/imgui/imgui/"
-- Flecs
IncludeDir["FLECS"]       = LumoraDir .. "/External/flecs/flecs/include"
-- Misc
IncludeDir["VISITSTRUCT"] = LumoraDir .. "/External/visit_struct/include"
IncludeDir["FILEWATCH"]   = LumoraDir .. "/External/filewatch/"

LumoraIncludeDirs =
{
    IncludeDir.LUMORA,
    IncludeDir.VULKAN,
    IncludeDir.GLAD,
    IncludeDir.GLFW,
    IncludeDir.LUA,
    IncludeDir.SOL,
    IncludeDir.GLM,
    IncludeDir.SPDLOG,
    IncludeDir.IMGUI,
    IncludeDir.FLECS,
    IncludeDir.VISITSTRUCT,
    IncludeDir.FILEWATCH,
}

-- Libraries
Library = {}
Library["VULKAN"] = "vulkan-1"
-- GLSL to SPIR-V at runtime (Vulkan backend). Shared, not shaderc_combined: the SDK ships shaderc
-- release-CRT only, so a static link is an LNK2038 mismatch against the /MDd Debug build.
Library["SHADERC"] = "shaderc_shared"
-- Windows
Library["Win"] = {}
Library["Win"]["GDI32"]    = "gdi32"
Library["Win"]["KERNEL32"] = "kernel32"
Library["Win"]["PSAPI"]    = "psapi"
-- Linux
Library["Linux"] = {}
Library["Linux"]["DL"]      = "dl"
Library["Linux"]["GL"]      = "GL"
Library["Linux"]["X11"]     = "X11"
Library["Linux"]["PTHREAD"] = "pthread"
-- MacOS
Library["MacOS"] = {}
Library["MacOS"]["QUARTZ"]    = "QuartzCore.framework"
Library["MacOS"]["COCOA"]     = "Cocoa.framework"
Library["MacOS"]["IOKIT"]     = "IOKit.framework"
Library["MacOS"]["METAL"]     = "Metal.framework"
Library["MacOS"]["COREVIDEO"] = "CoreVideo.framework"
