cmake_minimum_required(VERSION 3.14)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(spatia_guide LANGUAGES CXX)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(../.. spatia-build)
endif ()

add_executable(spatia_guide main.cpp)
target_link_libraries(spatia_guide PRIVATE spatia::spatia)
target_compile_features(spatia_guide PRIVATE cxx_std_23)
