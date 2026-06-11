#==============================================================================
# src/cmd/CMakeLists.txt
# Command line application (wpengine)
#==============================================================================

cmake_minimum_required(VERSION 3.12)

# Source files
set(CMD_SOURCES
    main.cpp
)

if(DEMOMODE)
    list(APPEND CMD_SOURCES
        recording.cpp
        recording.h
    )
endif()

# Create executable (use different target name to avoid conflict with library)
add_executable(wpengine-cli
    ${CMD_SOURCES}
)

# Link against the core library (wpengine)
target_link_libraries(wpengine-cli PRIVATE
    wpengine
)

# Compile features
target_compile_features(wpengine-cli PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(wpengine-cli PRIVATE /FS)
endif()

# Set output name to wpengine.exe and output directory
set_target_properties(wpengine-cli PROPERTIES
    OUTPUT_NAME "wpengine-cli"
    RUNTIME_OUTPUT_DIRECTORY ${TARGET_OUTPUT_DIRECTORY}
)
