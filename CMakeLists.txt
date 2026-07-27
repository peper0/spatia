cmake_minimum_required(VERSION 3.14)

project(spatia VERSION 0.1.0 LANGUAGES CXX)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# The library is header-only.
add_library(spatia INTERFACE)
add_library(spatia::spatia ALIAS spatia)
target_compile_features(spatia INTERFACE cxx_std_23)
target_include_directories(
        spatia
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Installation and packaging
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
        DIRECTORY include/spatia
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
        TARGETS spatia
        EXPORT spatia-targets
)
install(
        EXPORT spatia-targets
        NAMESPACE spatia::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spatia
)
configure_package_config_file(
        cmake/spatia-config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/spatia-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spatia
)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/spatia-config-version.cmake"
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT
)
install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/spatia-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/spatia-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spatia
)

option(BUILD_EXAMPLES "Build spatia examples" ON)
if (BUILD_EXAMPLES)
    add_subdirectory(examples/guide)
endif ()

option(BUILD_DOCS "Build Sphinx and Doxygen documentation" OFF)
if (BUILD_DOCS)
    add_subdirectory(docs)
endif ()

# Compile-time performance tests
#
# Optional and not part of the release: each benchmarks/compose_variants/
# <variant>/compose.hpp is an alternative implementation of the transform
# graph, built from the same sources to compare build times. The whole section
# is skipped when those directories are absent.
#
# Run all variants sequentially with:
#
#   ./scripts/benchmark_compile_times.sh
#
# Or build just one, for example:
#
#   cmake --build build/compile-time \
#       --target spatia_compile_time_tests_variant_a_opt_large_graph
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/compose_variants" AND
    EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_time")
file(
        GLOB SPATIA_COMPOSE_VARIANT_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/compose_variants/*/compose.hpp"
)
list(SORT SPATIA_COMPOSE_VARIANT_HEADERS)

find_program(SPATIA_TIME_EXECUTABLE NAMES time)
if (NOT SPATIA_TIME_EXECUTABLE)
    message(WARNING "The 'time' program was not found; max RSS will be unavailable")
endif ()
set(SPATIA_COMPILE_TIME_METRICS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/compile_time_metrics")
file(MAKE_DIRECTORY "${SPATIA_COMPILE_TIME_METRICS_DIRECTORY}")
set(
        SPATIA_COMPILE_TIME_SCENARIOS
        large_graph
        many_uses
        four_trees_unrolled
        four_trees_generated
)
set(
        SPATIA_COMPILE_TIME_SOURCE_large_graph
        "tests/compile_time/transforms_compile_time_large_graph.cpp"
)
set(
        SPATIA_COMPILE_TIME_SOURCE_many_uses
        "tests/compile_time/transforms_compile_time_many_uses.cpp"
)
set(
        SPATIA_COMPILE_TIME_SOURCE_four_trees_unrolled
        "tests/compile_time/transforms_compile_time_four_trees_unrolled.cpp"
)
set(
        SPATIA_COMPILE_TIME_SOURCE_four_trees_generated
        "tests/compile_time/transforms_compile_time_four_trees_generated.cpp"
)

foreach (VARIANT_HEADER IN LISTS SPATIA_COMPOSE_VARIANT_HEADERS)
    get_filename_component(VARIANT_DIRECTORY "${VARIANT_HEADER}" DIRECTORY)
    get_filename_component(VARIANT_NAME "${VARIANT_DIRECTORY}" NAME)

    foreach (SCENARIO IN LISTS SPATIA_COMPILE_TIME_SCENARIOS)
        set(VARIANT_TARGET "spatia_compile_time_tests_${VARIANT_NAME}_${SCENARIO}")
        set(VARIANT_SOURCE "${SPATIA_COMPILE_TIME_SOURCE_${SCENARIO}}")

        add_library(
                "${VARIANT_TARGET}"
                OBJECT
                EXCLUDE_FROM_ALL
                "${VARIANT_SOURCE}"
        )
        target_include_directories(
                "${VARIANT_TARGET}"
                BEFORE PRIVATE
                "${VARIANT_DIRECTORY}"
        )
        if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options("${VARIANT_TARGET}" PRIVATE -ftemplate-depth=8192)
        endif ()

        if (SPATIA_TIME_EXECUTABLE)
            set(VARIANT_METRICS_FILE "${SPATIA_COMPILE_TIME_METRICS_DIRECTORY}/${VARIANT_NAME}_${SCENARIO}.csv")
            set_property(
                    TARGET "${VARIANT_TARGET}"
                    PROPERTY CXX_COMPILER_LAUNCHER "${SPATIA_TIME_EXECUTABLE};--format=%e,%M;--output=${VARIANT_METRICS_FILE}"
            )
        else ()
            set_property(
                    TARGET "${VARIANT_TARGET}"
                    PROPERTY CXX_COMPILER_LAUNCHER "${CMAKE_COMMAND};-E;time"
            )
        endif ()
    endforeach ()
endforeach ()
endif ()

# Runtime benchmarks
#
# Configure and run with:
#
#   cmake -S . -B build/benchmark \
#       -DBUILD_TESTS=OFF \
#       -DBUILD_BENCHMARKS=ON \
#       -DCMAKE_BUILD_TYPE=Release
#   cmake --build build/benchmark --target spatia_inline_benchmark
#   ./build/benchmark/spatia_inline_benchmark
option(BUILD_BENCHMARKS "Build Google Benchmark microbenchmarks" OFF)
if (BUILD_BENCHMARKS)
    find_package(benchmark CONFIG QUIET)

    if (NOT TARGET benchmark::benchmark_main)
        include(FetchContent)
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
                googlebenchmark
                GIT_REPOSITORY https://github.com/google/benchmark.git
                GIT_TAG v1.9.5
                GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(googlebenchmark)
    endif ()

    add_executable(
            spatia_inline_benchmark
            benchmarks/transforms_inline_benchmark.cpp
    )
    target_link_libraries(
            spatia_inline_benchmark
            PRIVATE
            spatia
            benchmark::benchmark_main
    )
endif ()

# Add tests
option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_executable(
            spatia_tests
            tests/transforms_tests.cpp
            tests/coordinate_systems/coordinate_systems_tests.cpp
            tests/coordinate_systems/rotation_algebra_tests.cpp
            tests/projective_tests.cpp
            tests/spec_api_tests.cpp
    )
    target_link_libraries(spatia_tests PRIVATE spatia GTest::GTest GTest::Main)
    add_test(NAME spatia_tests COMMAND spatia_tests)
    if (TARGET spatia_guide)
        add_test(NAME spatia_guide_smoke COMMAND spatia_guide)
    endif ()
endif ()
