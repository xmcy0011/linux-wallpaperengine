#==============================================================================
# src/demo_win32/CMakeLists.txt
# Windows Win32 demo application
#==============================================================================

cmake_minimum_required(VERSION 3.12)

project(DemoWin32UI)

# Source files
set(DEMO_SOURCES
    demo_win32.cpp
    demo_win32_ui.cpp
)

# Create executable (WIN32 to avoid console window)
add_executable(${PROJECT_NAME} WIN32
    ${DEMO_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Link against the WallpaperEngine library
target_link_libraries(${PROJECT_NAME} PRIVATE
    wpengine
)

# Windows libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        gdi32
        comdlg32
        shell32
        comctl32
    )
endif()

# Compile features
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /FS)
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TARGET_OUTPUT_DIRECTORY}
)

# MFC demo (optional)
if(BUILD_MFC_DEMO)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/demo_mfc mfc-demo)
    add_dependencies(mfc-demo wpengine)
endif()
