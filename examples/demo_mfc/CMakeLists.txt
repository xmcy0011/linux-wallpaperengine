cmake_minimum_required(VERSION 3.20)
project(DemoMFC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find MFC
if(MSVC)
    # Enable MFC
    set(CMAKE_MFC_FLAG 2)  # Static MFC
    add_definitions(-D_AFXDLL -D_UNICODE -D_UNICODE)
endif()

# Source files
set(SRC_FILES
    DemoMFC.cpp
    MainFrm.cpp
    ControlPanel.cpp
)

# Header files
set(HEADER_FILES
    stdafx.h
    DemoMFC.h
    MainFrm.h
    ControlPanel.h
    resource.h
)

# Resource file
set(RC_FILES
    demo_mfc.rc
)

# Create executable
add_executable(${PROJECT_NAME} WIN32
    ${SRC_FILES}
    ${HEADER_FILES}
    ${RC_FILES}
)

# Precompiled header
if(MSVC)
    target_precompile_headers(${PROJECT_NAME} PRIVATE stdafx.h)
endif()

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
        winspool
        comdlg32
        advapi32
        shell32
        ole32
        oleaut32
        uuid32
        odbc32
        odbccp32
        comctl32
    )
endif()

# Set subsystem to Windows
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/output
)
