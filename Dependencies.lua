-- Lumora Dependencies
vk_sdk = os.getenv("VULKAN_SDK")
if (vk_sdk == nil) then
    print("VULKAN_SDK environment variable not set. Please install the Vulkan SDK and set the VULKAN_SDK environment variable.")
    os.exit(1)
end
VULKAN_LIB_DIR = vk_sdk .. "/Lib"

IncludeDir = {}
IncludeDir["VULKAN"] = vk_sdk .. "/Include"
IncludeDir["GLAD"] = "%{wks.location}/External/glad/include"
-- Windowing
IncludeDir["GLFW"] = "%{wks.location}/External/glfw/glfw/include"
-- Lua
IncludeDir["LUA"] = "%{wks.location}/External/lua/lua/"
IncludeDir["SOL"] = "%{wks.location}/External/sol/include/"
-- Maths
IncludeDir["GLM"] = "%{wks.location}/External/glm/"
-- Logging
IncludeDir["SPDLOG"] = "%{wks.location}/External/spdlog/include"
-- ImGui
IncludeDir["IMGUI"] = "%{wks.location}/External/imgui/imgui/"
-- Flecs
IncludeDir["FLECS"] = "%{wks.location}/External/flecs/flecs/include"
-- Visit Struct
IncludeDir["VISITSTRUCT"] = "%{wks.location}/External/visit_struct/include"
IncludeDir["FILEWATCH"] = "%{wks.location}/External/filewatch/"


-- Libraries
Library = {}
Library["VULKAN"] = "vulkan-1"
-- Windows
Library["Win"] = {}
Library["Win"]["GDI32"] = "gdi32"
Library["Win"]["KERNEL32"] = "kernel32"
Library["Win"]["PSAPI"] = "psapi"
-- Linux
Library["Linux"] = {}
Library["Linux"]["DL"] = "dl"
Library["Linux"]["GL"] = "GL"
Library["Linux"]["X11"] = "X11"
Library["Linux"]["PTHREAD"] = "pthread"
-- MacOS
Library["MacOS"] = {}
Library["MacOS"]["QUARTZ"] = "QuartzCore.framework"
Library["MacOS"]["COCOA"] = "Cocoa.framework"
Library["MacOS"]["IOKIT"] = "IOKit.framework"
Library["MacOS"]["METAL"] = "Metal.framework"
Library["MacOS"]["COREVIDEO"] = "CoreVideo.framework"