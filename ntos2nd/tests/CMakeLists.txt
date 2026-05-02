# Tests CMakeLists.txt

# Simple test suite
add_executable(simple_test simple_test.c)
target_include_directories(simple_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(simple_test PRIVATE ntos2nd)

# Add tests
add_test(NAME SimpleTest COMMAND simple_test)
set_tests_properties(SimpleTest PROPERTIES TIMEOUT 30)