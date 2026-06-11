#==============================================================================
# src/WallpaperEngine/CMakeLists.txt
# Core WallpaperEngine library
#==============================================================================

cmake_minimum_required(VERSION 3.12)

#==============================================================================
# Source files
#==============================================================================

# Core library sources
set(WALLPAPERENGINE_SOURCES
    # Steam/FileSystem
    ${CMAKE_CURRENT_SOURCE_DIR}/../Steam/FileSystem/FileSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Steam/FileSystem/FileSystem.h

    # Logging
    Logging/Log.cpp
    Logging/Log.h

    # Debugging
    Debugging/CallStack.cpp
    Debugging/CallStack.h

    # Application
    Application/ApplicationContext.cpp
    Application/ApplicationContext.h
    Application/WallpaperApplication.cpp
    Application/WallpaperApplication.h

    # Assets
    Assets/AssetLoadException.cpp
    Assets/AssetLoadException.h
    Assets/AssetLocator.cpp
    Assets/AssetLocator.h

    # FileSystem
    FileSystem/Container.h
    FileSystem/Container.cpp
    FileSystem/Utf8Path.h
    FileSystem/Adapters/Types.h
    FileSystem/Adapters/Directory.cpp
    FileSystem/Adapters/Directory.h
    FileSystem/Adapters/Package.cpp
    FileSystem/Adapters/Package.h
    FileSystem/Adapters/Virtual.cpp
    FileSystem/Adapters/Virtual.h

    # Audio
    Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.cpp
    Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.h
    Audio/Drivers/Recorders/PlaybackRecorder.cpp
    Audio/Drivers/Recorders/PlaybackRecorder.h
    Audio/Drivers/Detectors/PulseAudioPlayingDetector.cpp
    Audio/Drivers/Detectors/PulseAudioPlayingDetector.h
    Audio/Drivers/Detectors/AudioPlayingDetector.cpp
    Audio/Drivers/Detectors/AudioPlayingDetector.h
    Audio/Drivers/AudioDriver.cpp
    Audio/Drivers/AudioDriver.h
    Audio/Drivers/SDLAudioDriver.cpp
    Audio/Drivers/SDLAudioDriver.h
    Audio/AudioContext.cpp
    Audio/AudioContext.h
    Audio/AudioStream.cpp
    Audio/AudioStream.h

    # Input
    Input/InputContext.cpp
    Input/InputContext.h
    Input/MouseInput.h
    Input/Drivers/GLFWMouseInput.cpp
    Input/Drivers/GLFWMouseInput.h

    # Render/Shaders/Variables
    Render/Shaders/Variables/ShaderVariable.h
    Render/Shaders/Variables/ShaderVariable.cpp
    Render/Shaders/Variables/ShaderVariableFloat.h
    Render/Shaders/Variables/ShaderVariableFloat.cpp
    Render/Shaders/Variables/ShaderVariableInteger.h
    Render/Shaders/Variables/ShaderVariableInteger.cpp
    Render/Shaders/Variables/ShaderVariableVector2.h
    Render/Shaders/Variables/ShaderVariableVector2.cpp
    Render/Shaders/Variables/ShaderVariableVector3.h
    Render/Shaders/Variables/ShaderVariableVector3.cpp
    Render/Shaders/Variables/ShaderVariableVector4.h
    Render/Shaders/Variables/ShaderVariableVector4.cpp

    # Render/Shaders
    Render/Shaders/Shader.h
    Render/Shaders/Shader.cpp
    Render/Shaders/ShaderUnit.cpp
    Render/Shaders/ShaderUnit.h
    Render/Shaders/GLSLContext.cpp
    Render/Shaders/GLSLContext.h

    # Render/Helpers
    Render/Helpers/ContextAware.cpp
    Render/Helpers/ContextAware.h

    # Render/Drivers
    Render/Drivers/VideoFactories.cpp
    Render/Drivers/VideoFactories.h
    Render/Drivers/Detectors/FullScreenDetector.cpp
    Render/Drivers/Detectors/FullScreenDetector.h
    Render/Drivers/Output/Output.cpp
    Render/Drivers/Output/Output.h
    Render/Drivers/Output/GLFWWindowOutput.cpp
    Render/Drivers/Output/GLFWWindowOutput.h
    Render/Drivers/Output/GLFWOutputViewport.cpp
    Render/Drivers/Output/GLFWOutputViewport.h
    Render/Drivers/Output/OutputViewport.cpp
    Render/Drivers/Output/OutputViewport.h
    Render/Drivers/GLFWOpenGLDriver.h
    Render/Drivers/GLFWOpenGLDriver.cpp
    Render/Drivers/VideoDriver.h
    Render/Drivers/VideoDriver.cpp
    Render/RenderContext.h
    Render/RenderContext.cpp
    Render/TextureCache.h
    Render/TextureCache.cpp
    Render/FBOProvider.cpp
    Render/FBOProvider.h

    # Render core
    Render/CWallpaper.h
    Render/CWallpaper.cpp
    Render/WallpaperState.h
    Render/WallpaperState.cpp
    Render/Wallpapers/CScene.h
    Render/Wallpapers/CScene.cpp
    Render/Wallpapers/CVideo.h
    Render/Wallpapers/CVideo.cpp
    Render/Camera.h
    Render/Camera.cpp
    Render/CObject.h
    Render/CObject.cpp
    Render/CTexture.cpp
    Render/CTexture.h
    Render/TextureProvider.h

    # Render/Objects
    Render/Objects/CRenderable.cpp
    Render/Objects/CRenderable.h
    Render/Objects/CImage.h
    Render/Objects/CImage.cpp
    Render/Objects/CSound.h
    Render/Objects/CSound.cpp
    Render/Objects/CParticle.h
    Render/Objects/CParticle.cpp
    Render/Objects/CText.h
    Render/Objects/CText.cpp

    # Render/CFBO and Effects
    Render/CFBO.h
    Render/CFBO.cpp
    Render/Objects/Effects/CPass.h
    Render/Objects/Effects/CPass.cpp

    # Render/Text
    Render/Text/FontAtlas.h
    Render/Text/FontAtlas.cpp
    Render/Text/Utf8.h
    Render/Text/Utf8.cpp

    # Data
    Data/Assets/Types.h
    Data/Assets/Texture.h
    Data/Assets/Package.h
    Data/Model/Types.h
    Data/Model/Project.h
    Data/Model/Wallpaper.h
    Data/Model/Object.h
    Data/Model/Material.h
    Data/Model/Effect.h
    Data/Model/Model.h
    Data/Model/UserSetting.h
    Data/Model/DynamicValue.h
    Data/Model/DynamicValue.cpp
    Data/Model/Property.h
    Data/Utils/TypeCaster.h
    Data/Utils/BinaryReader.cpp
    Data/Utils/BinaryReader.h
    Data/Utils/MemoryStream.h
    Data/Utils/SFINAE.h
    Data/Parsers/EffectParser.cpp
    Data/Parsers/EffectParser.h
    Data/Parsers/ProjectParser.cpp
    Data/Parsers/ProjectParser.h
    Data/Parsers/WallpaperParser.cpp
    Data/Parsers/WallpaperParser.h
    Data/Parsers/UserSettingParser.cpp
    Data/Parsers/UserSettingParser.h
    Data/Parsers/ObjectParser.cpp
    Data/Parsers/ObjectParser.h
    Data/Parsers/MaterialParser.cpp
    Data/Parsers/MaterialParser.h
    Data/Parsers/ModelParser.cpp
    Data/Parsers/ModelParser.h
    Data/Parsers/ShaderConstantParser.cpp
    Data/Parsers/ShaderConstantParser.h
    Data/Parsers/PackageParser.cpp
    Data/Parsers/PackageParser.h
    Data/Parsers/PropertyParser.cpp
    Data/Parsers/PropertyParser.h
    Data/Parsers/TextureParser.cpp
    Data/Parsers/TextureParser.h
    Data/Builders/UserSettingBuilder.h
    Data/Builders/VectorBuilder.cpp
    Data/Builders/VectorBuilder.h
    Data/Dumpers/StringPrinter.cpp
    Data/Dumpers/StringPrinter.h
    Data/JSON.cpp

    # VideoPlayback
    VideoPlayback/MPV/GLPlayer.cpp
    VideoPlayback/MPV/GLPlayer.h
    VideoPlayback/MPV/MemoryStreamProtocol.cpp
    VideoPlayback/MPV/MemoryStreamProtocol.h

    # Scripting
    Scripting/ScriptEngine.h
    Scripting/ScriptEngine.cpp
    Data/Model/ScriptedDynamicValue.h
    Data/Model/ScriptedDynamicValue.cpp
)

# Wayland sources
if(WAYLAND_SUPPORT_FOUND)
    list(APPEND WALLPAPERENGINE_SOURCES
        Render/Drivers/WaylandOpenGLDriver.h
        Render/Drivers/WaylandOpenGLDriver.cpp
        Render/Drivers/Detectors/WaylandFullScreenDetector.cpp
        Render/Drivers/Detectors/WaylandFullScreenDetector.h
        Render/Drivers/Output/WaylandOutput.cpp
        Render/Drivers/Output/WaylandOutput.h
        Render/Drivers/Output/WaylandOutputViewport.cpp
        Render/Drivers/Output/WaylandOutputViewport.h
        Input/Drivers/WaylandMouseInput.cpp
        Input/Drivers/WaylandMouseInput.h
        ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.c
        ${WAYLAND_OUTPUT_DIR}/xdg-shell-protocol.h
        ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.c
        ${WAYLAND_OUTPUT_DIR}/wlr-foreign-toplevel-management-unstable-v1-protocol.h
        ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.c
        ${WAYLAND_OUTPUT_DIR}/wlr-layer-shell-unstable-v1-protocol.h
    )
endif()

# X11 sources
if(X11_SUPPORT_FOUND)
    list(APPEND WALLPAPERENGINE_SOURCES
        Render/Drivers/Output/X11Output.cpp
        Render/Drivers/Output/X11Output.h
        Render/Drivers/Detectors/X11FullScreenDetector.cpp
        Render/Drivers/Detectors/X11FullScreenDetector.h
    )
endif()

# Demo mode sources
if(DEMOMODE)
    list(APPEND WALLPAPERENGINE_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../recording.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../recording.h
    )
endif()

# Conditional source removal based on options
if(NOT ENABLE_MPV)
    list(REMOVE_ITEM WALLPAPERENGINE_SOURCES
        Render/Wallpapers/CVideo.h
        Render/Wallpapers/CVideo.cpp
        VideoPlayback/MPV/GLPlayer.cpp
        VideoPlayback/MPV/GLPlayer.h
        VideoPlayback/MPV/MemoryStreamProtocol.cpp
        VideoPlayback/MPV/MemoryStreamProtocol.h
    )
endif()

if(NOT ENABLE_PULSEAUDIO)
    list(REMOVE_ITEM WALLPAPERENGINE_SOURCES
        Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.cpp
        Audio/Drivers/Recorders/PulseAudioPlaybackRecorder.h
        Audio/Drivers/Detectors/PulseAudioPlayingDetector.cpp
        Audio/Drivers/Detectors/PulseAudioPlayingDetector.h
    )
endif()

#==============================================================================
# External helper sources (MimeTypes)
#==============================================================================

set(MIMETYPES_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../External/MimeTypes/MimeTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../External/MimeTypes/MimeTypes.h
)

#==============================================================================
# Targets
#==============================================================================

# Shared library (wpengine-lib)
# Note: On Windows, we use wpengine DLL instead. This is for non-Windows platforms.
if(NOT WIN32)
    add_library(wpengine-lib SHARED
        ${WALLPAPERENGINE_SOURCES}
        ${MIMETYPES_SOURCES}
    )

    # Include directories
    target_include_directories(wpengine-lib PUBLIC
        ${WALLPAPERENGINE_INCLUDE_DIRS}
    )

    # Link libraries
    target_link_libraries(wpengine-lib PUBLIC
        ${GLM_LINK_TARGET}
        ${OPENGL_LIBRARIES}
        GLEW::GLEW
        Freetype::Freetype
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
        qjs
    )

    if(ENABLE_MPV)
        target_link_libraries(wpengine-lib PUBLIC ${MPV_LIBRARY})
    endif()

    if(ENABLE_PULSEAUDIO)
        target_link_libraries(wpengine-lib PUBLIC ${PULSEAUDIO_LIBRARY})
    endif()

    # Compile features
    target_compile_features(wpengine-lib PUBLIC cxx_std_20)

    if(MSVC)
        target_compile_options(wpengine-lib PRIVATE /FS)
    endif()

    # Compile definitions
    target_compile_definitions(wpengine-lib PUBLIC
        ERRORONLY=${ERRORONLY}
        DEMOMODE=${DEMOMODE}
    )

    if(ENABLE_MPV)
        target_compile_definitions(wpengine-lib PUBLIC WALLPAPERENGINE_ENABLE_MPV=1)
    else()
        target_compile_definitions(wpengine-lib PUBLIC WALLPAPERENGINE_ENABLE_MPV=0)
    endif()

    if(ENABLE_PULSEAUDIO)
        target_compile_definitions(wpengine-lib PUBLIC WALLPAPERENGINE_ENABLE_PULSEAUDIO=1)
    else()
        target_compile_definitions(wpengine-lib PUBLIC WALLPAPERENGINE_ENABLE_PULSEAUDIO=0)
    endif()

    if(X11_SUPPORT_FOUND)
        target_compile_definitions(wpengine-lib PUBLIC ENABLE_X11)
        check_function_exists(XSetIOErrorExitHandler HAVE_XSETIOERROREXITHANDLER)
        if(HAVE_XSETIOERROREXITHANDLER)
            target_compile_definitions(wpengine-lib PUBLIC HAVE_XSETIOERROREXITHANDLER=1)
        endif()
    endif()

    if(WAYLAND_SUPPORT_FOUND)
        target_compile_definitions(wpengine-lib PUBLIC ENABLE_WAYLAND)
    endif()

    if(WAYLAND_SUPPORT_FOUND AND NOT X11_SUPPORT_FOUND)
        target_compile_definitions(wpengine-lib PUBLIC GLEW_EGL)
    endif()

    # Output directory
    set_target_properties(wpengine-lib PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${TARGET_OUTPUT_DIRECTORY}
    )

    # Add dependencies
    add_dependencies(wpengine-lib glslang spirv-cross-core spirv-cross-glsl)
endif()

#==============================================================================
# Windows DLL target
#==============================================================================

if(WIN32)
    # Create the DLL library
    add_library(wpengine SHARED
        Engine/Engine.cpp
        API/CAPI.cpp
        ${WALLPAPERENGINE_SOURCES}
        ${MIMETYPES_SOURCES}
    )

    # Set Windows export all symbols
    set_target_properties(wpengine PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )

    # Compile definitions
    target_compile_definitions(wpengine PRIVATE
        WALLPAPERENGINE_EXPORTS
        UNICODE
        _UNICODE
        ERRORONLY=${ERRORONLY}
        DEMOMODE=${DEMOMODE}
    )

    # Include directories
    target_include_directories(wpengine PUBLIC
        ${WALLPAPERENGINE_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
    )

    # Link libraries for DLL
    target_link_libraries(wpengine PUBLIC
        ${GLM_LINK_TARGET}
        ${OPENGL_LIBRARIES}
        GLEW::GLEW
        Freetype::Freetype
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
        qjs
        Dbghelp
    )

    if(ENABLE_MPV)
        target_link_libraries(wpengine PUBLIC ${MPV_LIBRARY})
        target_compile_definitions(wpengine PUBLIC WALLPAPERENGINE_ENABLE_MPV=1)
    else()
        target_compile_definitions(wpengine PUBLIC WALLPAPERENGINE_ENABLE_MPV=0)
    endif()

    if(ENABLE_PULSEAUDIO)
        target_link_libraries(wpengine PUBLIC ${PULSEAUDIO_LIBRARY})
        target_compile_definitions(wpengine PUBLIC WALLPAPERENGINE_ENABLE_PULSEAUDIO=1)
    else()
        target_compile_definitions(wpengine PUBLIC WALLPAPERENGINE_ENABLE_PULSEAUDIO=0)
    endif()

    # Target compile features
    target_compile_features(wpengine PUBLIC cxx_std_20)
    if(MSVC)
        target_compile_options(wpengine PRIVATE /FS)
    endif()

    # Platform-specific defines
    if(X11_SUPPORT_FOUND)
        target_compile_definitions(wpengine PUBLIC ENABLE_X11)
        if(HAVE_XSETIOERROREXITHANDLER)
            target_compile_definitions(wpengine PUBLIC HAVE_XSETIOERROREXITHANDLER=1)
        endif()
    endif()

    if(WAYLAND_SUPPORT_FOUND)
        target_compile_definitions(wpengine PUBLIC ENABLE_WAYLAND)
    endif()

    if(WAYLAND_SUPPORT_FOUND AND NOT X11_SUPPORT_FOUND)
        target_compile_definitions(wpengine PUBLIC GLEW_EGL)
    endif()

    # Set output directories
    set_target_properties(wpengine PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${TARGET_OUTPUT_DIRECTORY}
        RUNTIME_OUTPUT_DIRECTORY ${TARGET_OUTPUT_DIRECTORY}
        ARCHIVE_OUTPUT_DIRECTORY ${TARGET_OUTPUT_DIRECTORY}
    )

    # Add dependencies
    add_dependencies(wpengine glslang spirv-cross-core spirv-cross-glsl)
endif()
