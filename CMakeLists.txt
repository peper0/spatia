cmake_minimum_required(VERSION 3.10)

# Project name
project(geometry2)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add the library
add_library(geometry2 src/geometry2.cpp)

# Include directories1
target_include_directories(geometry2 PUBLIC include)

# Add tests
# Link only the targets provided by FindGTest. GMock target may not be available
# depending on the system package. Remove GMock link unless tests use gmock.
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
	enable_testing()
	find_package(GTest REQUIRED)
	add_executable(geometry2_tests tests/geometry2a_tests.cpp)
	target_link_libraries(geometry2_tests PRIVATE geometry2 GTest::GTest GTest::Main)
	add_test(NAME geometry2_tests COMMAND geometry2_tests)
endif()