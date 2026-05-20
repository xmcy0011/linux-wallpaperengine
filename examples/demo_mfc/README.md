# Wallpaper Engine DLL Demo - MFC Application

An interactive MFC application for testing the Wallpaper Engine DLL API on Windows.

## Features

- **Interactive UI**: Visual interface with controls for all DLL API functions
- **Property Editing**: View and modify wallpaper properties in real-time
- **Playback Control**: Play, Pause, Stop buttons
- **Volume Control**: Volume slider and mute checkbox
- **Screenshot**: Capture current wallpaper frame
- **Settings**: Adjust maximum FPS
- **Window Embedding**: Demonstrates window handle injection for embedded rendering

## Building

### Prerequisites

- Visual Studio 2019 or later with MFC support
- CMake 3.20 or later
- Wallpaper Engine DLL build

### Build Steps

```bash
# From project root
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_MFC_DEMO=ON ..
cmake --build . --config Release
```

### Using Visual Studio

1. Open the CMakeLists.txt in Visual Studio
2. Select the `DemoMFC` target
3. Build and Run

## Usage

1. **Set Assets Path**: Go to `File > Set Assets Path` and select the Wallpaper Engine assets folder
2. **Load Wallpaper**: Go to `File > Load Wallpaper` and select a wallpaper folder
3. **Control Playback**: Use the control panel on the right to:
   - Play/Pause/Stop playback
   - Adjust volume
   - Modify properties
   - Take screenshots

## API Functions Demonstrated

- `WE_Create()` / `WE_Destroy()`
- `WE_SetAssetsPath()`
- `WE_LoadWallpaper()`
- `WE_SetWindowHandle()` - Window embedding
- `WE_Play()` / `WE_Pause()` / `WE_Stop()`
- `WE_IsPlaying()` / `WE_IsPaused()`
- `WE_ListProperties()` / `WE_SetProperty()` / `WE_GetProperty()`
- `WE_SetVolume()` / `WE_SetMuted()`
- `WE_SetMaxFPS()`
- `WE_TakeScreenshot()`
- `WE_GetLastError()`

## Architecture

```
DemoMFC.exe
    ├── CDemoMFCApp (Application)
    │   └── WE_Engine* (Engine instance)
    ├── CMainFrame (Main Frame Window)
    │   ├── m_hWallpaperWnd (Embedded wallpaper window)
    │   └── m_wndControlPanel (Control Panel)
    └── CControlPanelBar (Control Panel)
        └── Property Controls, Playback Controls, etc.
```

## Window Embedding

The app demonstrates window handle injection:

1. Creates a child window (`m_hWallpaperWnd`)
2. Passes the HWND to `WE_SetWindowHandle()`
3. The GLFW window is automatically embedded using `SetParent()` API

## Configuration

Settings are saved to `demo_mfc.ini`:
- `AssetsPath`: Last used assets folder
- `WallpaperPath`: Last loaded wallpaper
