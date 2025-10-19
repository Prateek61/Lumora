return {
    Name = "Lumora Application",
    AssetsDirectory = "",
    WindowProps = {
        Title = "Lumora",
        Width = 1280,
        Height = 720,
    },

    Run = true, -- Setting this to false doesn't run the application loop

    LoggerConfig = {
        File = "../Lumora.log",
        Core = { Level = "trace" }, -- levels: [trace, debug, info, warn, error, fatal, off]
        Client = { Level = "trace" },
        CoreSerializer = { Level = "trace" }
    }
}