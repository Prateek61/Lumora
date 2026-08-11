return {
    WindowSettings = {
        Title = "Silly Linguine Cat Simulator Deluxe Online",
        Width = 1280,
        Height = 720,
        VSync = true,
    },
    ImGuiSettings = {
        DockingEnabled = true,
        ViewportsEnabled = true,
        ShowDemoWindow = false,
        DockSpaceOverMainViewport = true,
        IniFilename = "ImGui.local.ini",
    },
    RenderAPI = "Vulkan", -- Vulkan, OpenGL
    UseGlimmer = true,
    GlimmerImGui = false,
    DiagnosticPlugin = true,
}