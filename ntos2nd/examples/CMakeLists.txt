# Examples CMakeLists.txt

# Test program that runs inside the sandbox
add_executable(test_program test_program.c)
target_include_directories(test_program PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Simple demo program
add_executable(demo_program demo_program.c)
target_include_directories(demo_program PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})