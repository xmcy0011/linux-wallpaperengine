cmake_minimum_required(VERSION 3.20)
project(DemoWin32UI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SRC_FILES
    DemoWin32UI.cpp
)

# Create executable
add_executable(${PROJECT_NAME} WIN32
    ${SRC_FILES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# Link against the WallpaperEngine DLL library
target_link_libraries(${PROJECT_NAME} PRIVATE
    linux-wallpaperengine-dll
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

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output
)
