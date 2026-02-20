cmake_minimum_required(VERSION 3.20)
project(druk-installer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

add_executable(druk-installer WIN32
    src/main.cpp
    src/ui_window.cpp
    src/ui_wizard.cpp
    src/ui_theme.cpp
    src/logic_engine.cpp
    src/logic_system.cpp
    src/logic_rollback.cpp
    installer.rc
)

# Custom command to zip the compiler assets before building the installer
add_custom_command(
    TARGET druk-installer PRE_BUILD
    COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/package.ps1" -SourceDir "${CMAKE_CURRENT_SOURCE_DIR}/../.." -DestZip "${CMAKE_CURRENT_SOURCE_DIR}/payload.zip"
    COMMENT "Packaging compiler assets into payload.zip with correct directory structure"
)

target_include_directories(druk-installer PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(druk-installer PRIVATE
    gdiplus
    shlwapi
    comctl32
)

set_target_properties(druk-installer PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
