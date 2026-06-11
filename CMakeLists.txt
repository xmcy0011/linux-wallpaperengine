cmake_minimum_required(VERSION 3.12)
project(wpengine VERSION 1.0.0 LANGUAGES C CXX)

# prevent installing to /usr by default so we don't bork anything
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/opt/${PROJECT_NAME}" CACHE PATH "..." FORCE)
endif()

set_property(GLOBAL PROPERTY OS_FOLDERS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
set(OpenGL_GL_PREFERENCE "LEGACY")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(ENABLE_MPV "Link libmpv for video wallpapers and video textures (disable for scene-only builds)" ON)
if(WIN32)
    option(ENABLE_PULSEAUDIO "PulseAudio integration (Linux desktop audio capture / auto-mute)" OFF)
else()
    option(ENABLE_PULSEAUDIO "PulseAudio integration (Linux desktop audio capture / auto-mute)" ON)
endif()
option(BUILD_WIN32_UI_DEMO "Build Win32 UI demo application" ON)
option(BUILD_MFC_DEMO "Build MFC demo application (requires MFC component)" OFF)

# Demo mode
if(NOT DEMOMODE)
    set(DEMOMODE 0)
endif()

# Error only mode
if(NOT ERRORONLY)
    set(ERRORONLY 0)
endif()

# Output directories - don't set global CMAKE_RUNTIME_OUTPUT_DIRECTORY
# to prevent external dependency executables from cluttering our output
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)
set(TARGET_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)

# PIC settings
set(ENABLE_OPT OFF)
set(SPIRV_CROSS_FORCE_PIC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable unnecessary outputs from external dependencies
set(KISSFFT_TEST OFF CACHE BOOL "Build kissfft tests (unused by this project)" FORCE)
set(KISSFFT_TOOLS OFF CACHE BOOL "Build kissfft CLI tools (unused by this project)" FORCE)

# QuickJS - disable unnecessary outputs
set(QJS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(QJS_BUILD_CLI_STATIC OFF CACHE BOOL "" FORCE)
set(QJS_BUILD_CLI_WITH_MIMALLOC OFF CACHE BOOL "" FORCE)
set(QJS_BUILD_CLI_WITH_STATIC_MIMALLOC OFF CACHE BOOL "" FORCE)

# glslang - disable standalone tools and tests
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)

# SPIRV-Cross - disable tests and CLI
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL OFF CACHE BOOL "" FORCE)
set(SPIRV_TOOLS_BUILD_STATIC OFF CACHE BOOL "" FORCE)

#==============================================================================
# Dependencies
#==============================================================================

find_package(X11)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLUT REQUIRED)
find_package(ZLIB REQUIRED)

# SDL2
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    find_package(SDL2 MODULE REQUIRED)
endif()
if(SDL2_FOUND AND NOT SDL2_LIBRARIES)
    if(TARGET SDL2::SDL2)
        set(SDL2_LIBRARIES SDL2::SDL2)
    elseif(TARGET SDL2::SDL2-static)
        set(SDL2_LIBRARIES SDL2::SDL2-static)
    endif()
endif()
if(SDL2_FOUND AND NOT SDL2_INCLUDE_DIRS)
    if(TARGET SDL2::SDL2)
        get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
    elseif(TARGET SDL2::SDL2-static)
        get_target_property(SDL2_INCLUDE_DIRS SDL2::SDL2-static INTERFACE_INCLUDE_DIRECTORIES)
    endif()
endif()

# MPV
if(ENABLE_MPV)
    if(WIN32)
        if(NOT EXISTS "E:/repo/github/mpv-dev/include" OR NOT EXISTS "E:/repo/github/mpv-dev/libmpv.lib")
            message(FATAL_ERROR "MPV development files not found in E:/repo/github/mpv-dev. Please install mpv-dev and ensure the include and libmpv.lib files are present.")
        endif()
        set(MPV_INCLUDE_DIR "E:/repo/github/mpv-dev/include")
        set(MPV_LIBRARY "E:/repo/github/mpv-dev/libmpv.lib")
    else()
        find_package(MPV REQUIRED)
    endif()
else()
    message(STATUS "ENABLE_MPV is OFF: video wallpapers and video textures are disabled")
    set(MPV_INCLUDE_DIR "")
    set(MPV_LIBRARY "")
endif()

find_package(LZ4 REQUIRED)
find_package(FFMPEG REQUIRED)

# GLFW
if(WIN32)
    find_package(glfw3 CONFIG REQUIRED)
else()
    find_package(glfw3 CONFIG QUIET)
endif()
if(TARGET glfw3::glfw)
    set(GLFW_LINK_TARGET glfw3::glfw)
elseif(TARGET glfw)
    set(GLFW_LINK_TARGET glfw)
else()
    set(GLFW_LINK_TARGET glfw)
endif()

# GLM (header-only)
find_package(glm CONFIG QUIET)
set(GLM_GLOBAL_INCLUDE "")
if(glm_FOUND AND TARGET glm::glm-header-only)
    get_target_property(GLM_GLOBAL_INCLUDE glm::glm-header-only INTERFACE_INCLUDE_DIRECTORIES)
elseif(glm_FOUND AND TARGET glm::glm)
    get_target_property(GLM_GLOBAL_INCLUDE glm::glm INTERFACE_INCLUDE_DIRECTORIES)
endif()
if(NOT GLM_GLOBAL_INCLUDE OR GLM_GLOBAL_INCLUDE STREQUAL "GLM_GLOBAL_INCLUDE-NOTFOUND")
    find_path(GLM_INCLUDE_DIR NAMES glm/glm.hpp REQUIRED
        DOC "Directory containing glm/glm.hpp (e.g. install libglm-dev or vcpkg glm)")
    set(GLM_GLOBAL_INCLUDE "${GLM_INCLUDE_DIR}")
    if(NOT TARGET glm::glm-header-only)
        add_library(glm::glm-header-only INTERFACE IMPORTED GLOBAL)
        target_include_directories(glm::glm-header-only INTERFACE "${GLM_INCLUDE_DIR}")
    endif()
endif()
include_directories(BEFORE "${GLM_GLOBAL_INCLUDE}")

if(TARGET glm::glm-header-only)
    set(GLM_LINK_TARGET glm::glm-header-only)
elseif(TARGET glm::glm)
    set(GLM_LINK_TARGET glm::glm)
else()
    set(GLM_LINK_TARGET glm::glm-header-only)
endif()

# PulseAudio
if(ENABLE_PULSEAUDIO)
    find_package(PulseAudio REQUIRED)
else()
    message(STATUS "ENABLE_PULSEAUDIO is OFF: PulseAudio recorder/detector disabled (no libpulse)")
    set(PULSEAUDIO_INCLUDE_DIR "")
    set(PULSEAUDIO_LIBRARY "")
endif()

#==============================================================================
# External dependencies
#==============================================================================

# Disable unnecessary outputs from glslang
set(GLSLANG_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)

# Disable unnecessary outputs from SPIRV-Cross
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL OFF CACHE BOOL "" FORCE)

# Disable unnecessary outputs from QuickJS
set(QJS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(QJS_DISABLE_BUILDTIME_LOOKUP_TABLE OFF CACHE BOOL "" FORCE)

add_subdirectory(src/External/glslang-WallpaperEngine glslang)
add_subdirectory(src/External/SPIRV-Cross-WallpaperEngine spirv-cross)
add_subdirectory(src/External/kissfft kissfft)
add_subdirectory(src/External/argparse argparse)
add_subdirectory(src/External/quickjs quickjs)
if(BUILD_TESTING)
    add_subdirectory(src/External/Catch2)
endif()

#==============================================================================
# Wayland support
#==============================================================================

pkg_check_modules(WAYLAND_SUPPORT wayland-cursor wayland-protocols egl wayland-egl)

if(WAYLAND_SUPPORT_FOUND)
    find_program(WaylandScanner NAMES wayland-scanner)
    message(STATUS "Found WaylandScanner at ${WaylandScanner}")
    execute_process(
        COMMAND pkg-config --variable=pkgdatadir wayland-protocols
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Found wayland-protocols at ${WAYLAND_PROTOCOLS_DIR}")
    set(WAYLAND_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/wayland)

    file(MAKE_DIRECTORY ${WAYLAND_OUTPUT_DIR})

    message(STATUS "Building protocols...")
    add_custom_command(OUTPUT ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.h
        COMMAND ${WaylandScanner} client-header ${CMAKE_CURRENT_SOURCE_DIR}/protocols/wlr-foreign-toplevel-management-unstable-v1.xml ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.h)
    add_custom_command(OUTPUT ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.c
        COMMAND ${WaylandScanner} private-code ${CMAKE_CURRENT_SOURCE_DIR}/protocols/wlr-foreign-toplevel-management-unstable-v1.xml ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.c)
    add_custom_command(OUTPUT ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.h
        COMMAND ${WaylandScanner} client-header ${CMAKE_CURRENT_SOURCE_DIR}/protocols/wlr-layer-shell-unstable-v1.xml ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.h)
    add_custom_command(OUTPUT ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.c
        COMMAND ${WaylandScanner} private-code ${CMAKE_CURRENT_SOURCE_DIR}/protocols/wlr-layer-shell-unstable-v1.xml ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.c)
    add_custom_command(OUTPUT ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.h
        COMMAND ${WaylandScanner} client-header ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.h)
    add_custom_command(OUTPUT ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.c
        COMMAND ${WaylandScanner} private-code ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.c)

    set(WAYLAND_LIBRARIES
        pthread
        wayland-cursor
        wayland-client
        wayland-egl
        ${OPENGL_egl_LIBRARY})
endif()

#==============================================================================
# X11 support
#==============================================================================

if(X11_FOUND)
    if(NOT X11_Xrandr_FOUND)
        message(WARNING "X11 support disabled. Xrandr package is missing")
        set(X11_SUPPORT_FOUND FALSE)
    elseif(NOT X11_xf86vmode_FOUND)
        message(WARNING "X11 support disabled. Xxf86vm package is missing")
        set(X11_SUPPORT_FOUND FALSE)
    else()
        message("X11 support enabled")
        set(X11_SUPPORT_FOUND TRUE)
        set(X11_LIBRARIES ${X11_LIBRARIES} ${X11_Xrandr_LIB})
        set(X11_INCLUDES ${X11_INCLUDE_DIR} ${XRANDR_INCLUDE_DIR})
        set(CMAKE_REQUIRED_LIBRARIES ${X11_LIBRARIES})
    endif()
endif()

#==============================================================================
# Warning checks
#==============================================================================

if(DEMOMODE)
    message(WARNING "Enabling demo mode will automatically record 5 seconds and stop the software. This is used internally to produce the video seen on the website as a sort of status report")
endif()

if(NOT WAYLAND_SUPPORT_FOUND AND NOT X11_SUPPORT_FOUND)
    message(WARNING "No window server detected at build time. You will only be able to preview backgrounds")
endif()

#==============================================================================
# Main project directories
#==============================================================================

add_subdirectory(src)

#==============================================================================
# Installation
#==============================================================================

install(DIRECTORY ${TARGET_OUTPUT_DIRECTORY}/ DESTINATION .)
install(FILES ${TARGET_OUTPUT_DIRECTORY}/${PROJECT_NAME} PERMISSIONS OWNER_READ OWNER_WRITE WORLD_EXECUTE WORLD_READ GROUP_READ DESTINATION .)
