cmake_minimum_required(VERSION 3.16)
project(LibreNTInstaller VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect cross-compilation for Windows using mingw-w64
if(CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32|g\\+\\+-mingw")
    set(MINGW_CROSS_COMPILE TRUE)
    message(STATUS "Detected mingw-w64 cross-compilation")
endif()

# Windows-specific settings (native or cross-compiled)
if(WIN32 OR MINGW_CROSS_COMPILE)
    add_definitions(-DUNICODE -D_UNICODE -DWIN32 -D_WINDOWS)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/ImGuiImpl.cpp
    src/InstallerUI.cpp
    src/VerboseBoot.cpp
    src/DeploymentEngine.cpp
    src/RegistryPatcher.cpp
    src/DriverInjector.cpp
)

# Header files
set(HEADERS
    include/ImGuiImpl.h
    include/InstallerUI.h
    include/VerboseBoot.h
    include/DeploymentEngine.h
    include/RegistryPatcher.h
    include/DriverInjector.h
)

# Create executable
add_executable(LibreNTInstaller ${SOURCES} ${HEADERS})

# Windows libraries (native Windows or mingw-w64 cross-compilation)
if(WIN32 OR MINGW_CROSS_COMPILE)
    target_link_libraries(LibreNTInstaller PRIVATE
        user32
        gdi32
        dxgi
        d3d11
        dwmapi
        ole32
        oleaut32
        comdlg32
        version
        setupapi
        cfgmgr32
    )
endif()

# FetchContent for ImGui
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# ImGui source files (without main.cpp)
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

target_link_libraries(LibreNTInstaller PRIVATE imgui)

# Windows resources (only for native Windows build, not cross-compile)
if(WIN32 AND NOT MINGW_CROSS_COMPILE)
    target_sources(LibreNTInstaller PRIVATE ${CMAKE_SOURCE_DIR}/resources.rc)
endif()

# Install targets
install(TARGETS LibreNTInstaller RUNTIME DESTINATION bin)
