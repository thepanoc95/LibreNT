cmake_minimum_required(VERSION 3.15)
project(ntos2nd VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Detect mingw-w64 cross-compiler
if(NOT DEFINED CMAKE_C_COMPILER AND EXISTS "/usr/bin/x86_64-w64-mingw32-gcc")
    # Try to detect mingw-w64 cross-compiler
    find_program(MINGW_GCC x86_64-w64-mingw32-gcc)
    if(MINGW_GCC)
        message(STATUS "Detected mingw-w64 cross-compiler: ${MINGW_GCC}")
        set(CMAKE_C_COMPILER ${MINGW_GCC})
    endif()
endif()

# Cross-compilation detection
if(CMAKE_C_COMPILER MATCHES "mingw" OR CMAKE_C_COMPILER MATCHES "w64-mingw32")
    set(CROSS_COMPILE_MINGW TRUE)
    message(STATUS "Cross-compiling with mingw-w64 for Windows")
endif()

# MSVC-specific settings
if(MSVC)
    add_compile_options(/W4 /Od /Zi)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
endif()

# MinGW/GCC settings (native or cross-compilation)
if(MINGW OR CROSS_COMPILE_MINGW)
    if(CROSS_COMPILE_MINGW)
        # Cross-compilation flags for Windows target
        add_compile_options(-Wall -Wextra -O2)
        add_compile_definitions(WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
        add_compile_definitions(_WIN32_WINNT=0x0601)  # Windows 7 minimum
    else()
        add_compile_options(-Wall -Wextra -g)
    endif()
endif()

# Source files
set(NTOS2ND_SOURCES
    ntos2nd/kernel.c
    ntos2nd/syscall.c
    ntos2nd/apibridge.c
    ntos2nd/sandbox.c
    ntos2nd/undocumented.c
    ntos2nd/ntos2nd.c
    MSWindows.c
    NTCall.c
)

# Header files
set(NTOS2ND_HEADERS
    ntos2nd/kernel.h
    ntos2nd/syscall.h
    ntos2nd/apibridge.h
    ntos2nd/sandbox.h
    ntos2nd/undocumented.h
    NTCall.h
)

# Create the kernel DLL
add_library(ntos2nd SHARED ${NTOS2ND_SOURCES} ${NTOS2ND_HEADERS})

target_include_directories(ntos2nd PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link Windows libraries
target_link_libraries(ntos2nd PRIVATE
    kernel32
    user32
    ntdll
    psapi
)

# Install targets
install(TARGETS ntos2nd
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)

install(FILES ${NTOS2ND_HEADERS}
    DESTINATION include/ntos2nd
)

# Main executable
add_executable(ntos2ndk NTMain.c)
target_include_directories(ntos2ndk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ntos2ndk PRIVATE ntos2nd)

# Example programs
add_subdirectory(examples)

# Tests
enable_testing()
add_subdirectory(tests)

# WinELF - Rust ELF loader subproject
find_package(Cargo QUIET)
if(Cargo_FOUND)
    add_custom_target(winelf
        COMMAND cargo build --release
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/winelf
        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/winelf/target/release/winelf-loader.exe
        COMMENT "Building WinELF in release mode"
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/winelf/target/release/winelf-loader.exe
        DESTINATION bin
    )
endif()


# POSIX subsystem runtime DLL (user mode)
add_library(posixsubsystem SHARED
    posix/runtime/posixsubsystem.c
    posix/runtime/posixsubsystem.def
    posix/include/posix_abi.h
)
target_include_directories(posixsubsystem PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/posix/include
)
target_link_libraries(posixsubsystem PRIVATE
    kernel32
)
install(TARGETS posixsubsystem
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)
install(FILES posix/include/posix_abi.h
    DESTINATION include/ntos2nd/posix
)
