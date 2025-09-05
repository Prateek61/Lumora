-- Lumora Dependencies

IncludeDir = {}
IncludeDir["BGFX"] = "%{wks.location}/External/bgfx/bgfx/include"
IncludeDir["BX"] = "%{wks.location}/External/bx/bx/include"
IncludeDir["BIMG"] = "%{wks.location}/External/bimg/bimg/include"
IncludeDir["GLFW"] = "%{wks.location}/External/glfw/glfw/include"

-- Libraries
Library = {}
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

-- BGFX Compactability
function setBxCompat()
    filter "action:vs*"
        includedirs { path.join(IncludeDir["BX"], "compat/msvc") }
        buildoptions { "/Zc:__cplusplus", "/Zc:preprocessor" }
    filter { "system:windows", "action:gmake*" }
        includedirs { path.join(IncludeDir["BX"], "compat/mingw") }
    filter { "system:macosx" }
        includedirs { path.join(IncludeDir["BX"], "compat/osx") }
        buildoptions { "-x objective-c++" }
    filter {}
end