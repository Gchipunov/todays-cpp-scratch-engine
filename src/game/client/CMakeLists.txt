cmake_minimum_required(VERSION 3.10)
project(IntMainClientDLL)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(CMAKE_CXX_STANDARD 11)

# Add ImGui source files
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
)

# Add your DLL source
add_library(ImGuiDLL SHARED
    dllmain.c
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(ImGuiDLL PRIVATE
    ${CMAKE_SOURCE_DIR}/imgui
)
