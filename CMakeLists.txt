cmake_minimum_required(VERSION 3.12)
project(linux-wallpaperengine)

# prevent installing to /usr by default so we don't bork anything
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/opt/${PROJECT_NAME}" CACHE PATH "..." FORCE)
endif()

set_property(GLOBAL PROPERTY OS_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
set(OpenGL_GL_PREFERENCE "LEGACY")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_MPV "Link libmpv for video wallpapers and video textures (disable for scene-only builds)" ON)

# puts the app in demomode, which records a 5 second webm video and quits
if(NOT DEMOMODE)
    set(DEMOMODE 0)
endif()

# if you're developing you might find this debug option useful for shader output, although RenderDoc is encouraged
if(NOT ERRORONLY)
    set(ERRORONLY 0)
endif()

find_package(X11)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

find_package(GLUT REQUIRED)
find_package(ZLIB REQUIRED)
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
if(ENABLE_MPV)
    find_package(MPV REQUIRED)
else()
    message(STATUS "ENABLE_MPV is OFF: video wallpapers and video textures are disabled")
    set(MPV_INCLUDE_DIR "")
    set(MPV_LIBRARY "")
endif()
find_package(LZ4 REQUIRED)
find_package(FFMPEG REQUIRED)

# GLFW: on Windows plain "glfw" becomes glfw.lib (wrong); vcpkg provides CONFIG target (glfw / glfw3::glfw).
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

# GLM (header-only): WallpaperEngine + vendored SPIRV-Cross include <glm/...>. Linux often has
# system headers; MSVC + vcpkg needs the glm port and include path before External subdirs build.
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

if(WIN32)
    option(ENABLE_PULSEAUDIO "PulseAudio integration (Linux desktop audio capture / auto-mute)" OFF)
else()
    option(ENABLE_PULSEAUDIO "PulseAudio integration (Linux desktop audio capture / auto-mute)" ON)
endif()
if(ENABLE_PULSEAUDIO)
    find_package(PulseAudio REQUIRED)
else()
    message(STATUS "ENABLE_PULSEAUDIO is OFF: PulseAudio recorder/detector disabled (no libpulse)")
    set(PULSEAUDIO_INCLUDE_DIR "")
    set(PULSEAUDIO_LIBRARY "")
endif()

set(ENABLE_OPT OFF)
set(SPIRV_CROSS_FORCE_PIC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# kissfft uses option() with default ON; a stale cache can re-enable tools and require libpng (pkg-config).
set(KISSFFT_TEST OFF CACHE BOOL "Build kissfft tests (unused by this project)" FORCE)
set(KISSFFT_TOOLS OFF CACHE BOOL "Build kissfft CLI tools (unused by this project)" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(TARGET_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)

add_subdirectory(src/External/glslang-WallpaperEngine glslang)
add_subdirectory(src/External/SPIRV-Cross-WallpaperEngine spirv-cross)
add_subdirectory(src/External/kissfft kissfft)
add_subdirectory(src/External/argparse argparse)
set(QJS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(src/External/quickjs quickjs)
if(BUILD_TESTING)
    add_subdirectory(src/External/Catch2)
endif()
# try to enable wayland builds when possible
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

    include_directories(${WAYLAND_SUPPORT_INCLUDE_DIRS})
    include_directories(${WAYLAND_OUTPUT_DIR})
    set(WAYLAND_LIBRARIES
        pthread
        wayland-cursor
        wayland-client
        wayland-egl
        ${OPENGL_egl_LIBRARY})
    set(WAYLAND_SOURCES
        src/WallpaperEngine/Render/Drivers/WaylandOpenGLDriver.h
        src/WallpaperEngine/Render/Drivers/WaylandOpenGLDriver.cpp
        src/WallpaperEngine/Render/Drivers/Detectors/WaylandFullScreenDetector.cpp
        src/WallpaperEngine/Render/Drivers/Detectors/WaylandFullScreenDetector.h
        src/WallpaperEngine/Render/Drivers/Output/WaylandOutput.cpp
        src/WallpaperEngine/Render/Drivers/Output/WaylandOutput.h
        src/WallpaperEngine/Render/Drivers/Output/WaylandOutputViewport.cpp
        src/WallpaperEngine/Render/Drivers/Output/WaylandOutputViewport.h
        src/WallpaperEngine/Input/Drivers/WaylandMouseInput.cpp
        src/WallpaperEngine/Input/Drivers/WaylandMouseInput.h
        ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.c
        ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.h
        ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.c
        ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.h
        ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.c
        ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.h)
endif()

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
    endif()
endif()

if(X11_SUPPORT_FOUND)
    include_directories(${X11_INCLUDE_DIR} ${XRANDR_INCLUDE_DIR})

    set(X11_LIBRARIES
        ${X11_LIBRARIES}
        ${X11_Xrandr_LIB})
    set(X11_SOURCES
        src/WallpaperEngine/Render/Drivers/Output/X11Output.cpp
        src/WallpaperEngine/Render/Drivers/Output/X11Output.h
        src/WallpaperEngine/Render/Drivers/Detectors/X11FullScreenDetector.cpp
        src/WallpaperEngine/Render/Drivers/Detectors/X11FullScreenDetector.h)
    SET(X11_INCLUDES
        ${X11_INCLUDE_DIR}
        ${XRANDR_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${X11_LIBRARIES})
endif()

if(DEMOMODE)
    set(DEMOMODE_SOURCES
        src/recording.cpp
        src/recording.h)
    message(WARNING "Enabling demo mode will automatically record 5 seconds and stop the software. This is used internally to produce the video seen on the website as a sort of status report")
endif()
if(NOT WAYLAND_SUPPORT_FOUND AND NOT X11_SUPPORT_FOUND)
    message(WARNING "No window server detected at build time. You will only be able to preview backgrounds")
endif()

include_directories(
    src
    src/External/glslang-WallpaperEngine
    src/External/SPIRV-Cross-WallpaperEngine
    src/External/MimeTypes
    src/External/json/include
    src/External/stb
    src/External/kissfft
    ${MPV_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${LZ4_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIR}
    ${PULSEAUDIO_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${X11_INCLUDES})

set(COMMON_SOURCES
    src/External/MimeTypes/MimeTypes.cpp
    src/External/MimeTypes/MimeTypes.h

    src/Steam/FileSystem/FileSystem.cpp
    src/Steam/FileSystem/FileSystem.h

    src/WallpaperEngine/Logging/Log.cpp
    src/WallpaperEngine/Logging/Log.h

    src/WallpaperEngine/Debugging/CallStack.cpp
    src/WallpaperEngine/Debugging/CallStack.h

    src/WallpaperEngine/Application/ApplicationContext.cpp
    src/WallpaperEngine/Application/ApplicationContext.h
    src/WallpaperEngine/Application/WallpaperApplication.cpp
    src/WallpaperEngine/Application/WallpaperApplication.h

    src/WallpaperEngine/Assets/AssetLoadException.cpp
    src/WallpaperEngine/Assets/AssetLoadException.h
    src/WallpaperEngine/Assets/AssetLocator.cpp
    src/WallpaperEngine/Assets/AssetLocator.h

    src/WallpaperEngine/FileSystem/Container.h
    src/WallpaperEngine/FileSystem/Container.cpp
    src/WallpaperEngine/FileSystem/Utf8Path.h
    src/WallpaperEngine/FileSystem/Adapters/Types.h
    src/WallpaperEngine/FileSystem/Adapters/Directory.cpp
    src/WallpaperEngine/FileSystem/Adapters/Directory.h
    src/WallpaperEngine/FileSystem/Adapters/Package.cpp
    src/WallpaperEngine/FileSystem/Adapters/Package.h
    src/WallpaperEngine/FileSystem/Adapters/Virtual.cpp
    src/WallpaperEngine/FileSystem/Adapters/Virtual.h

    src/WallpaperEngine/Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.cpp
    src/WallpaperEngine/Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.h
    src/WallpaperEngine/Audio/Drivers/Recorders/PlaybackRecorder.cpp
    src/WallpaperEngine/Audio/Drivers/Recorders/PlaybackRecorder.h

    src/WallpaperEngine/Audio/Drivers/Detectors/PulseAudioPlayingDetector.cpp
    src/WallpaperEngine/Audio/Drivers/Detectors/PulseAudioPlayingDetector.h
    src/WallpaperEngine/Audio/Drivers/Detectors/AudioPlayingDetector.cpp
    src/WallpaperEngine/Audio/Drivers/Detectors/AudioPlayingDetector.h

    src/WallpaperEngine/Audio/Drivers/AudioDriver.cpp
    src/WallpaperEngine/Audio/Drivers/AudioDriver.h
    src/WallpaperEngine/Audio/Drivers/SDLAudioDriver.cpp
    src/WallpaperEngine/Audio/Drivers/SDLAudioDriver.h
    src/WallpaperEngine/Audio/AudioContext.cpp
    src/WallpaperEngine/Audio/AudioContext.h
    src/WallpaperEngine/Audio/AudioStream.cpp
    src/WallpaperEngine/Audio/AudioStream.h

    src/WallpaperEngine/Input/InputContext.cpp
    src/WallpaperEngine/Input/InputContext.h
    src/WallpaperEngine/Input/MouseInput.h
    src/WallpaperEngine/Input/Drivers/GLFWMouseInput.cpp
    src/WallpaperEngine/Input/Drivers/GLFWMouseInput.h

    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariable.h
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariable.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableFloat.h
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableFloat.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableInteger.h
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableInteger.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector2.h
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector2.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector3.h
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector3.cpp
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector4.h
    src/WallpaperEngine/Render/Shaders/Variables/ShaderVariableVector4.cpp

    src/WallpaperEngine/Render/Shaders/Shader.h
    src/WallpaperEngine/Render/Shaders/Shader.cpp
    src/WallpaperEngine/Render/Shaders/ShaderUnit.cpp
    src/WallpaperEngine/Render/Shaders/ShaderUnit.h
    src/WallpaperEngine/Render/Shaders/GLSLContext.cpp
    src/WallpaperEngine/Render/Shaders/GLSLContext.h

    src/WallpaperEngine/Render/Helpers/ContextAware.cpp
    src/WallpaperEngine/Render/Helpers/ContextAware.h

    src/WallpaperEngine/Render/Drivers/VideoFactories.cpp
    src/WallpaperEngine/Render/Drivers/VideoFactories.h

    src/WallpaperEngine/Render/Drivers/Detectors/FullScreenDetector.cpp
    src/WallpaperEngine/Render/Drivers/Detectors/FullScreenDetector.h

    src/WallpaperEngine/Render/Drivers/Output/Output.cpp
    src/WallpaperEngine/Render/Drivers/Output/Output.h
    src/WallpaperEngine/Render/Drivers/Output/GLFWWindowOutput.cpp
    src/WallpaperEngine/Render/Drivers/Output/GLFWWindowOutput.h
    src/WallpaperEngine/Render/Drivers/Output/GLFWOutputViewport.cpp
    src/WallpaperEngine/Render/Drivers/Output/GLFWOutputViewport.h
    src/WallpaperEngine/Render/Drivers/Output/OutputViewport.cpp
    src/WallpaperEngine/Render/Drivers/Output/OutputViewport.h
    src/WallpaperEngine/Render/Drivers/GLFWOpenGLDriver.h
    src/WallpaperEngine/Render/Drivers/GLFWOpenGLDriver.cpp
    src/WallpaperEngine/Render/Drivers/VideoDriver.h
    src/WallpaperEngine/Render/Drivers/VideoDriver.cpp
    src/WallpaperEngine/Render/RenderContext.h
    src/WallpaperEngine/Render/RenderContext.cpp
    src/WallpaperEngine/Render/TextureCache.h
    src/WallpaperEngine/Render/TextureCache.cpp
    src/WallpaperEngine/Render/FBOProvider.cpp
    src/WallpaperEngine/Render/FBOProvider.h

    src/WallpaperEngine/Render/Helpers/ContextAware.cpp
    src/WallpaperEngine/Render/Helpers/ContextAware.h

    src/WallpaperEngine/Render/CWallpaper.h
    src/WallpaperEngine/Render/CWallpaper.cpp
    src/WallpaperEngine/Render/WallpaperState.h
    src/WallpaperEngine/Render/WallpaperState.cpp
    src/WallpaperEngine/Render/Wallpapers/CScene.h
    src/WallpaperEngine/Render/Wallpapers/CScene.cpp
    src/WallpaperEngine/Render/Wallpapers/CVideo.h
    src/WallpaperEngine/Render/Wallpapers/CVideo.cpp
    src/WallpaperEngine/Render/Camera.h
    src/WallpaperEngine/Render/Camera.cpp
    src/WallpaperEngine/Render/CObject.h
    src/WallpaperEngine/Render/CObject.cpp
    src/WallpaperEngine/Render/CTexture.cpp
    src/WallpaperEngine/Render/CTexture.h
    src/WallpaperEngine/Render/TextureProvider.h

    src/WallpaperEngine/Render/Objects/CRenderable.cpp
    src/WallpaperEngine/Render/Objects/CRenderable.h
    src/WallpaperEngine/Render/Objects/CImage.h
    src/WallpaperEngine/Render/Objects/CImage.cpp
    src/WallpaperEngine/Render/Objects/CSound.h
    src/WallpaperEngine/Render/Objects/CSound.cpp
    src/WallpaperEngine/Render/Objects/CParticle.h
    src/WallpaperEngine/Render/Objects/CParticle.cpp

    src/WallpaperEngine/Render/CFBO.h
    src/WallpaperEngine/Render/CFBO.cpp
    src/WallpaperEngine/Render/Objects/Effects/CPass.h
    src/WallpaperEngine/Render/Objects/Effects/CPass.cpp

    src/WallpaperEngine/Data/Assets/Types.h
    src/WallpaperEngine/Data/Assets/Texture.h
    src/WallpaperEngine/Data/Assets/Package.h
    src/WallpaperEngine/Data/Model/Types.h
    src/WallpaperEngine/Data/Model/Project.h
    src/WallpaperEngine/Data/Model/Wallpaper.h
    src/WallpaperEngine/Data/Model/Object.h
    src/WallpaperEngine/Data/Model/Material.h
    src/WallpaperEngine/Data/Model/Effect.h
    src/WallpaperEngine/Data/Model/Model.h
    src/WallpaperEngine/Data/Model/UserSetting.h
    src/WallpaperEngine/Data/Model/DynamicValue.h
    src/WallpaperEngine/Data/Model/DynamicValue.cpp
    src/WallpaperEngine/Data/Model/Property.h
    src/WallpaperEngine/Data/Utils/TypeCaster.h
    src/WallpaperEngine/Data/Utils/BinaryReader.cpp
    src/WallpaperEngine/Data/Utils/BinaryReader.h
    src/WallpaperEngine/Data/Utils/MemoryStream.h
    src/WallpaperEngine/Data/Utils/SFINAE.h
    src/WallpaperEngine/Data/Parsers/EffectParser.cpp
    src/WallpaperEngine/Data/Parsers/EffectParser.h
    src/WallpaperEngine/Data/Parsers/ProjectParser.cpp
    src/WallpaperEngine/Data/Parsers/ProjectParser.h
    src/WallpaperEngine/Data/Parsers/WallpaperParser.cpp
    src/WallpaperEngine/Data/Parsers/WallpaperParser.h
    src/WallpaperEngine/Data/Parsers/UserSettingParser.cpp
    src/WallpaperEngine/Data/Parsers/UserSettingParser.h
    src/WallpaperEngine/Data/Parsers/ObjectParser.cpp
    src/WallpaperEngine/Data/Parsers/ObjectParser.h
    src/WallpaperEngine/Data/Parsers/MaterialParser.cpp
    src/WallpaperEngine/Data/Parsers/MaterialParser.h
    src/WallpaperEngine/Data/Parsers/ModelParser.cpp
    src/WallpaperEngine/Data/Parsers/ModelParser.h
    src/WallpaperEngine/Data/Parsers/ShaderConstantParser.cpp
    src/WallpaperEngine/Data/Parsers/ShaderConstantParser.h
    src/WallpaperEngine/Data/Parsers/PackageParser.cpp
    src/WallpaperEngine/Data/Parsers/PackageParser.h
    src/WallpaperEngine/Data/Parsers/PropertyParser.cpp
    src/WallpaperEngine/Data/Parsers/PropertyParser.h
    src/WallpaperEngine/Data/Parsers/TextureParser.cpp
    src/WallpaperEngine/Data/Parsers/TextureParser.h
    src/WallpaperEngine/Data/Builders/UserSettingBuilder.h
    src/WallpaperEngine/Data/Builders/VectorBuilder.cpp
    src/WallpaperEngine/Data/Builders/VectorBuilder.h
    src/WallpaperEngine/Data/Dumpers/StringPrinter.cpp
    src/WallpaperEngine/Data/Dumpers/StringPrinter.h
    src/WallpaperEngine/Data/JSON.cpp

    src/WallpaperEngine/VideoPlayback/MPV/GLPlayer.cpp
    src/WallpaperEngine/VideoPlayback/MPV/GLPlayer.h
    src/WallpaperEngine/VideoPlayback/MPV/MemoryStreamProtocol.cpp
    src/WallpaperEngine/VideoPlayback/MPV/MemoryStreamProtocol.h
    src/WallpaperEngine/Scripting/ScriptEngine.h
    src/WallpaperEngine/Scripting/ScriptEngine.cpp
    src/WallpaperEngine/Data/Model/ScriptedDynamicValue.h
    src/WallpaperEngine/Data/Model/ScriptedDynamicValue.cpp)

if(NOT ENABLE_MPV)
    list(REMOVE_ITEM COMMON_SOURCES
        src/WallpaperEngine/Render/Wallpapers/CVideo.h
        src/WallpaperEngine/Render/Wallpapers/CVideo.cpp
        src/WallpaperEngine/VideoPlayback/MPV/GLPlayer.cpp
        src/WallpaperEngine/VideoPlayback/MPV/GLPlayer.h
        src/WallpaperEngine/VideoPlayback/MPV/MemoryStreamProtocol.cpp
        src/WallpaperEngine/VideoPlayback/MPV/MemoryStreamProtocol.h)
endif()

if(NOT ENABLE_PULSEAUDIO)
    list(REMOVE_ITEM COMMON_SOURCES
        src/WallpaperEngine/Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.cpp
        src/WallpaperEngine/Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.h
        src/WallpaperEngine/Audio/Drivers/Detectors/PulseAudioPlayingDetector.cpp
        src/WallpaperEngine/Audio/Drivers/Detectors/PulseAudioPlayingDetector.h)
endif()

if(BUILD_TESTING)
    add_executable(
        tests

        ${COMMON_SOURCES}

        src/WallpaperEngine/Testing/Cases/Example.cpp
        src/WallpaperEngine/Testing/Render/TestingOpenGLDriver.cpp
        src/WallpaperEngine/Testing/Render/TestingOpenGLDriver.h
        src/WallpaperEngine/Testing/Input/TestingMouseInput.cpp
        src/WallpaperEngine/Testing/Input/TestingMouseInput.h
        src/WallpaperEngine/Testing/Harnesses/RenderHarness.cpp
        src/WallpaperEngine/Testing/Harnesses/RenderHarness.h
        src/WallpaperEngine/Testing/Cases/MouseCoordinates.cpp)
endif()

add_library(
    linux-wallpaperengine-lib SHARED

    ${COMMON_SOURCES}
    ${WAYLAND_SOURCES}
    ${X11_SOURCES}
    ${DEMOMODE_SOURCES})
    
if(WIN32)
    # MSVC omits the import .lib when the DLL has no exported symbols; this project does not use
    # __declspec(dllexport) on its API. Export object symbols so the implib is created for the .exe link.
    set_target_properties(linux-wallpaperengine-lib PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

target_link_libraries (linux-wallpaperengine-lib PUBLIC
    ${GLM_LINK_TARGET}
    ${OPENGL_LIBRARIES}
    GLEW::GLEW
    ${GLUT_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${LZ4_LIBRARY}
    ${SDL2_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${WAYLAND_LIBRARIES}
    ${X11_LIBRARIES}
    kissfft
    glslang
    spirv-cross-core
    spirv-cross-glsl
    ${GLFW_LINK_TARGET}
    argparse
    qjs)
if(WIN32)
    target_link_libraries(linux-wallpaperengine-lib PUBLIC Dbghelp)
endif()

if(ENABLE_MPV)
    target_link_libraries(linux-wallpaperengine-lib PUBLIC ${MPV_LIBRARY})
endif()
if(ENABLE_PULSEAUDIO)
    target_link_libraries(linux-wallpaperengine-lib PUBLIC ${PULSEAUDIO_LIBRARY})
endif()

target_compile_features(linux-wallpaperengine-lib PUBLIC cxx_std_20)
if(MSVC)
    # C1041: parallel cl.exe (/MP) shares one vc*.pdb; /FS forces synchronized PDB writes.
    target_compile_options(linux-wallpaperengine-lib PRIVATE /FS)
endif()

if (BUILD_TESTING)
    target_link_libraries (tests PRIVATE
        Catch2::Catch2WithMain
        ${GLM_LINK_TARGET}
        ${OPENGL_LIBRARIES}
        GLEW::GLEW
        ${GLUT_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${LZ4_LIBRARY}
        ${SDL2_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${WAYLAND_LIBRARIES}
        ${X11_LIBRARIES}
        kissfft
        glslang
        spirv-cross-core
        spirv-cross-glsl
        ${GLFW_LINK_TARGET}
        argparse
        qjs)
    if(WIN32)
        target_link_libraries(tests PRIVATE Dbghelp)
    endif()
    if(ENABLE_MPV)
        target_link_libraries(tests PRIVATE ${MPV_LIBRARY})
    endif()
    if(ENABLE_PULSEAUDIO)
        target_link_libraries(tests PRIVATE ${PULSEAUDIO_LIBRARY})
    endif()
    target_compile_features(tests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(tests PRIVATE /FS)
    endif()
endif()

if(ENABLE_MPV)
    target_compile_definitions(linux-wallpaperengine-lib PUBLIC WALLPAPERENGINE_ENABLE_MPV=1)
else()
    target_compile_definitions(linux-wallpaperengine-lib PUBLIC WALLPAPERENGINE_ENABLE_MPV=0)
endif()
if(ENABLE_PULSEAUDIO)
    target_compile_definitions(linux-wallpaperengine-lib PUBLIC WALLPAPERENGINE_ENABLE_PULSEAUDIO=1)
else()
    target_compile_definitions(linux-wallpaperengine-lib PUBLIC WALLPAPERENGINE_ENABLE_PULSEAUDIO=0)
endif()

target_compile_definitions(linux-wallpaperengine-lib PUBLIC ERRORONLY=${ERRORONLY})
target_compile_definitions(linux-wallpaperengine-lib PUBLIC DEMOMODE=${DEMOMODE})

# windows 使用 unicode 多字符集
target_compile_definitions(linux-wallpaperengine-lib PUBLIC "UNICODE" "_UNICODE")

if (BUILD_TESTING)
    # tests should give as much output as possible
    target_compile_definitions(tests PRIVATE ERRORONLY=0)
    target_compile_definitions(tests PRIVATE DEMOMODE=0)
    if(ENABLE_MPV)
        target_compile_definitions(tests PRIVATE WALLPAPERENGINE_ENABLE_MPV=1)
    else()
        target_compile_definitions(tests PRIVATE WALLPAPERENGINE_ENABLE_MPV=0)
    endif()
    if(ENABLE_PULSEAUDIO)
        target_compile_definitions(tests PRIVATE WALLPAPERENGINE_ENABLE_PULSEAUDIO=1)
    else()
        target_compile_definitions(tests PRIVATE WALLPAPERENGINE_ENABLE_PULSEAUDIO=0)
    endif()
endif()

if(X11_SUPPORT_FOUND)
    target_compile_definitions(linux-wallpaperengine-lib PUBLIC ENABLE_X11)

    # make sure some of the X11 functions we'll use are available
    check_function_exists(XSetIOErrorExitHandler HAVE_XSETIOERROREXITHANDLER)

    if(HAVE_XSETIOERROREXITHANDLER)
        target_compile_definitions(linux-wallpaperengine-lib PUBLIC HAVE_XSETIOERROREXITHANDLER=1)
    endif()
endif()

if(WAYLAND_SUPPORT_FOUND)
    target_compile_definitions(linux-wallpaperengine-lib PUBLIC ENABLE_WAYLAND)
endif()

if(WAYLAND_SUPPORT_FOUND AND NOT X11_SUPPORT_FOUND)
    target_compile_definitions(linux-wallpaperengine-lib PUBLIC GLEW_EGL)
endif()

target_include_directories(linux-wallpaperengine-lib PUBLIC
    src
    src/External/glslang-WallpaperEngine
    src/External/SPIRV-Cross-WallpaperEngine
    src/External/MimeTypes
    src/External/json/include
    src/External/stb
    src/External/kissfft
    ${MPV_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${LZ4_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIR}
    ${PULSEAUDIO_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${X11_INCLUDES}
    ${WAYLAND_OUTPUT_DIR})

set_target_properties(linux-wallpaperengine-lib PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${TARGET_OUTPUT_DIRECTORY})

add_dependencies(linux-wallpaperengine-lib glslang spirv-cross-core spirv-cross-glsl)

add_executable(linux-wallpaperengine
    src/main.cpp)

if(MSVC)
    target_compile_options(linux-wallpaperengine PRIVATE /FS)
endif()

target_compile_features(linux-wallpaperengine PRIVATE cxx_std_20)
target_link_libraries (linux-wallpaperengine PUBLIC linux-wallpaperengine-lib)

# set some install parameters if not in debug mode
install(DIRECTORY ${TARGET_OUTPUT_DIRECTORY}/ DESTINATION .)
install(FILES ${TARGET_OUTPUT_DIRECTORY}/${PROJECT_NAME} PERMISSIONS OWNER_READ OWNER_WRITE WORLD_EXECUTE WORLD_READ GROUP_READ DESTINATION .)
