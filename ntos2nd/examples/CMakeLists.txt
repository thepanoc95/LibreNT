# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     Project documentation
# COPYRIGHT:   Copyright 2026 Panoc95

add_executable(test_program test_program.c)
target_include_directories(test_program PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Simple demo program
add_executable(demo_program demo_program.c)
target_include_directories(demo_program PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})