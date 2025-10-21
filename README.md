# Lumora

Lumora is a simple C++ rendering engine built on top of BGFX, designed for quick iteration and ease of use, mostly for my personal projects.
> **Note:** Only developed and tested on Windows so far. Mac and Linux support may require minor tweaks.

## Features
- **Asset Management System**
    - Hot-reloadable assets with change detection
    - Type-safe asset handles and registries
    - Lua-based asset definitions and props serialization
- **Rendering**
    - BGFX integration for cross-API rendering (OpenGL, DirectX, Vulkan, etc.)
    - Optional ImGui integration for in-app UI
- **Application Framework**
    - Layered architecture for modular logic
    - Event system for input and window events
    - Window/context management
- **Scripting**
    - Lua support via sol2
    - Seamless C++ <-> Lua data exchange
- **Utilities**
    - Logging (spdlog), profiling

## Prerequisites
- C++20 compatible compiler
- Premake5
- Dependencies in `External/` (handled via git submodules)

## Getting Started
1. Clone the repository
    ```shell
    git clone https://github.com/Prateek61/Lumora.git --recurse-submodules
    ```

2. Build the project using premake
    ```shell
    premake5 gmake # Or any other supported generator
    # Or (Windows only):
    python Scripts/build_run.py --build --generator gmake
    ```
3. Compile
    ```shell
    # Windows only:
    python Scripts/build_run.py --compile --generator gmake --config Release
    ```

4. Run
    ```shell
    # Windows only:
    python Scripts/build_run.py --run --generator gmake
    ```

> Try `python Scripts/build_run.py --help` for more options (Windows only).
