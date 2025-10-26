return {
    Name = "Lumora Application",
    AssetsDirectory = "",
    WindowProps = {
        Title = "Lumora",
        Width = 1280,
        Height = 720,
    },
    API = "Vulkan", -- Options: [Default, OpenGL, DirectX11, DirectX12, Vulkan]
    Run = true, -- Setting this to false doesn't run the application loop

    LoggerConfig = {
        File = "../Lumora.log",
        ConsolePattern = "[%T] %^%n: %v [%s:%#] %$",
        Core = "trace", -- levels: [trace, debug, info, warn, error, fatal, off]
        CoreSerializer = "info",
        CoreAssets = "off",
        CoreRenderer = "trace",
        CoreBgfx = "trace",
        Client = "trace",
    }
}