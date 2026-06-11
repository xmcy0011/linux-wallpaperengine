#==============================================================================
# src/CMakeLists.txt
# Manages subdirectories in src/
#==============================================================================

# Global include directories for all targets in src/
set(WALLPAPERENGINE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/External/glslang-WallpaperEngine
    ${CMAKE_CURRENT_SOURCE_DIR}/External/SPIRV-Cross-WallpaperEngine
    ${CMAKE_CURRENT_SOURCE_DIR}/External/MimeTypes
    ${CMAKE_CURRENT_SOURCE_DIR}/External/json/include
    ${CMAKE_CURRENT_SOURCE_DIR}/External/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/External/kissfft
    ${MPV_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${LZ4_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIR}
    ${PULSEAUDIO_INCLUDE_DIR}
    ${X11_INCLUDES}
)

if(WAYLAND_SUPPORT_FOUND)
    list(APPEND WALLPAPERENGINE_INCLUDE_DIRS ${WAYLAND_OUTPUT_DIR})
endif()

# WallpaperEngine library (core engine)
add_subdirectory(WallpaperEngine)

# Command line application
add_subdirectory(cmd)

# Windows-specific demos
if(WIN32)
    add_subdirectory(demo_win32)
endif()

# Testing
if(BUILD_TESTING)
    add_subdirectory(WallpaperEngine/Testing)
endif()
