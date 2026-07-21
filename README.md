# Lumora

Lumora is a C++20 game engine built around a plugin architecture and an
entity-component-system core. It's a personal project, focused on quick
iteration and a clean, modular design.

> **Note:** Developed and tested on Windows. Linux/macOS paths exist in the
> build scripts but are not regularly verified.

## Features

The engine is organised into layers, each a plugin you opt into:

- **Core** — application loop, plugin system, smart pointers, threading.
- **Aether** — entity-component-system, built on [flecs](https://github.com/SanderMertens/flecs).
  Worlds, entities, systems, queries, and ordered execution phases.
- **Flux** — windowing and input, via GLFW.
- **Lumen** — rendering. A `RenderDevice` abstraction with an OpenGL backend
  (Vulkan port planned) and a batched 2D renderer (`Renderer2D`) for
  coloured and textured quads.
- **Glyph** — Dear ImGui integration, with docking and multi-viewport support.
- **Rune** — Lua scripting and serialization. Typed C++ ↔ Lua conversion
  backed by `visit_struct` reflection and [sol2](https://github.com/ThePhD/sol2).
- **Atlas** — asset management. Name-based asset handles, per-type loaders,
  and hot reloading driven by a filesystem watcher.

Plus logging (spdlog) and lightweight profiling.

### Demos

`LumoraApp` is the engine's dev harness. It ships two demo plugins, selected by
`UseGlimmer` in `Assets/Config.lua`:

- **Glimmer** — a particle / attractor / trail playground, with an optional ImGui
  control panel (`UseGlimmer = true`).
- **Spark** — an animated ripple grid plus an Atlas hot-reload demo
  (`UseGlimmer = false`, the shipped default).

## Prerequisites

- A C++20 compiler (MSVC / Visual Studio 2026 on Windows — that's what the helper
  script's `vs2026` generator and its hardcoded `VsDevCmd.bat` path assume).
- [Premake5](https://premake.github.io/).
- The [Vulkan SDK](https://vulkan.lunarg.com/) — the `VULKAN_SDK` environment
  variable must be set.
- Python 3 (only for the convenience build script).

## Getting Started

1. Clone with submodules:
   ```shell
   git clone https://github.com/Prateek61/Lumora.git --recurse-submodules
   ```

2. Generate the project files:
   ```shell
   premake5 vs2026        # or gmake2, xcode4, etc.
   ```

3. Build and run. On Windows the helper script wraps generate + compile + run:
   ```shell
   python Scripts/build_run.py --build --compile --run --config Debug --generator vs2026
   ```
   `--generator` accepts `gmake2` (the default) or `vs2026`.
   Otherwise build the generated solution/makefiles directly with your
   toolchain. `LumoraApp` is the startup project.

> Run `python Scripts/build_run.py --help` for all options.

## Using Lumora in your own project

Lumora is designed to be consumed as a git submodule. From your game's
premake workspace:

```lua
workspace "MyGame"
    architecture "x64"
    startproject "MyGame"
    configurations { "Debug", "Release", "Dist" }

include "Lumora/LumoraProjects.lua"   -- the submodule: engine + all deps
include "MyGame"
```

In your project, call `LinkLumora()` to wire up include directories and
links (see `LumoraApp/premake5.lua` as the reference template):

```lua
project "MyGame"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    files { "src/**.h", "src/**.cpp" }
    includedirs { "src" }

    LinkLumora()
```
