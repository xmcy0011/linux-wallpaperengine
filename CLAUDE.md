# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

**Standard build:**
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE='Release' ..
make
```
Built binaries go to `build/output/`.

**Build with tests:**
```bash
cmake -DCMAKE_BUILD_TYPE='Release' -DBUILD_TESTING=1 ..
make
# Run tests
./output/tests
```

**Build options:**
- `DENABLE_MPV=OFF` - Disable video wallpapers (reduces dependencies)
- `DENABLE_PULSEAUDIO=OFF` - Disable PulseAudio integration
- `DDEMOMODE=1` - Record 5 seconds and quit (for generating demo videos)

## Architecture Overview

This is a C++20 project that renders Wallpaper Engine wallpapers on Linux. It uses OpenGL 3.3+ for rendering and supports both X11 and Wayland.

### Core Components

**Application Layer** (`src/WallpaperEngine/Application/`)
- `ApplicationContext` - Parses CLI arguments and manages application settings (audio, rendering, mouse, screenshots, playlists)
- `WallpaperApplication` - Main application orchestrator that manages wallpapers, audio, render context, and fullscreen detection

**Wallpaper Types** (`src/WallpaperEngine/Render/Wallpapers/`)
- `CWallpaper` - Base class for all wallpapers, handles FBO management and render pass coordination
- `CScene` - OpenGL scene-based wallpapers with objects, cameras, effects, and shaders
- `CVideo` - Video wallpapers rendered via MPV

**File System** (`src/WallpaperEngine/FileSystem/`)
- `Container` - Virtual file system with mount points
- Adapters: `Directory` (real files), `Package` (Wallpaper Engine's .pkg files), `Virtual` (VFS layer)
- Wallpaper Engine assets are packaged in custom .pkg format (LZ4-compressed)

**Render System** (`src/WallpaperEngine/Render/`)
- `VideoDriver` - Abstract driver for platform-specific rendering
- Drivers: `GLFWOpenGLDriver` (cross-platform), `WaylandOpenGLDriver`, `X11Output`
- `Output`/`OutputViewport` - Abstraction for screen vs window rendering
- `Shaders/` - GLSL shader handling with glslang and SPIRV-Cross for cross-compilation
- `Objects/` - Scene objects: `CImage`, `CParticle`, `CText`, `CSound`, `Effects/CPass`

**Audio System** (`src/WallpaperEngine/Audio/`)
- `SDLAudioDriver` - Audio playback
- `PulseAudioPlaybackRecorder` - Desktop audio capture (for audio-reactive wallpapers)
- `AudioPlayingDetector` - Auto-mute when other apps play sound

**Scripting** (`src/WallpaperEngine/Scripting/`)
- QuickJS engine for dynamic wallpaper properties and effects

### Data Flow

1. Wallpaper assets (Steam Workshop ID or local path) are loaded via `Container`
2. `ProjectParser` reads `project.json` and builds a `Project` model
3. `CWallpaper::fromWallpaper()` creates the appropriate wallpaper type (Scene or Video)
4. Each frame: `WallpaperApplication::render()` → `CWallpaper::render()` → `CScene::renderFrame()` or `CVideo::renderFrame()`
5. Final output is drawn to the screen background (X11/Wayland) or window

### Platform-Specific Code

**X11**: Uses XRandr for screen management. Located in `src/WallpaperEngine/Render/Drivers/Output/X11Output.*`

**Wayland**: Uses `wlr-layer-shell-unstable-v1` protocol. Protocol files in `protocols/` are processed at build time by `wayland-scanner`.

### Key File Locations

- CLI argument parsing: `src/WallpaperEngine/Application/ApplicationContext.cpp::loadSettingsFromArgv()`
- Main render loop: `src/WallpaperEngine/Application/WallpaperApplication.cpp::show()`
- Scene object creation: `src/WallpaperEngine/Render/Wallpapers/CScene.cpp::createObject()`
- Shader compilation: `src/WallpaperEngine/Render/Shaders/`

### Assets Path Detection

The app automatically detects Wallpaper Engine assets from Steam installation directories:
- `~/.steam/steam/steamapps/common/wallpaper_engine`
- `~/.local/share/Steam/steamapps/common/wallpaper_engine`
- Flatpak/Snap variants

Fallback: `--assets-dir <path>` CLI option.
