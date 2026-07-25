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
add_library(geometry2 src/geometry2.cpp
		include/compose.hpp)

# Include directories1
target_include_directories(geometry2 PUBLIC include)

# Compile-time performance tests
#
# Every include/compose_variants/<variant>/compose.hpp gets a separate target
# built from the same source. Run all variants sequentially with:
#
#   ./scripts/benchmark_compile_times.sh
#
# Or build just one, for example:
#
#   cmake --build build/compile-time \
#       --target geometry2_compile_time_tests_variant_a_opt_large_graph
file(
	GLOB GEOMETRY2_COMPOSE_VARIANT_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/include/compose_variants/*/compose.hpp"
)
list(SORT GEOMETRY2_COMPOSE_VARIANT_HEADERS)

find_program(GEOMETRY2_TIME_EXECUTABLE NAMES time)
if(NOT GEOMETRY2_TIME_EXECUTABLE)
	message(WARNING "The 'time' program was not found; max RSS will be unavailable")
endif()
set(GEOMETRY2_COMPILE_TIME_METRICS_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/compile_time_metrics")
file(MAKE_DIRECTORY "${GEOMETRY2_COMPILE_TIME_METRICS_DIRECTORY}")
set(
	GEOMETRY2_COMPILE_TIME_SCENARIOS
	large_graph
	many_uses
	four_trees_unrolled
	four_trees_generated
)
set(
	GEOMETRY2_COMPILE_TIME_SOURCE_large_graph
	"tests/compile_time/transforms_compile_time_large_graph.cpp"
)
set(
	GEOMETRY2_COMPILE_TIME_SOURCE_many_uses
	"tests/compile_time/transforms_compile_time_many_uses.cpp"
)
set(
	GEOMETRY2_COMPILE_TIME_SOURCE_four_trees_unrolled
	"tests/compile_time/transforms_compile_time_four_trees_unrolled.cpp"
)
set(
	GEOMETRY2_COMPILE_TIME_SOURCE_four_trees_generated
	"tests/compile_time/transforms_compile_time_four_trees_generated.cpp"
)

foreach(VARIANT_HEADER IN LISTS GEOMETRY2_COMPOSE_VARIANT_HEADERS)
	get_filename_component(VARIANT_DIRECTORY "${VARIANT_HEADER}" DIRECTORY)
	get_filename_component(VARIANT_NAME "${VARIANT_DIRECTORY}" NAME)

	foreach(SCENARIO IN LISTS GEOMETRY2_COMPILE_TIME_SCENARIOS)
		set(VARIANT_TARGET "geometry2_compile_time_tests_${VARIANT_NAME}_${SCENARIO}")
		set(VARIANT_SOURCE "${GEOMETRY2_COMPILE_TIME_SOURCE_${SCENARIO}}")

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
			"${CMAKE_CURRENT_SOURCE_DIR}/include"
		)
		if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
			target_compile_options("${VARIANT_TARGET}" PRIVATE -ftemplate-depth=8192)
		endif()

		if(GEOMETRY2_TIME_EXECUTABLE)
			set(VARIANT_METRICS_FILE "${GEOMETRY2_COMPILE_TIME_METRICS_DIRECTORY}/${VARIANT_NAME}_${SCENARIO}.csv")
			set_property(
				TARGET "${VARIANT_TARGET}"
				PROPERTY CXX_COMPILER_LAUNCHER "${GEOMETRY2_TIME_EXECUTABLE};--format=%e,%M;--output=${VARIANT_METRICS_FILE}"
			)
		else()
			set_property(
				TARGET "${VARIANT_TARGET}"
				PROPERTY CXX_COMPILER_LAUNCHER "${CMAKE_COMMAND};-E;time"
			)
		endif()
	endforeach()
endforeach()

# Add tests
# Link only the targets provided by FindGTest. GMock target may not be available
# depending on the system package. Remove GMock link unless tests use gmock.
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
	enable_testing()
	find_package(GTest REQUIRED)
	add_executable(geometry2_tests tests/transforms_tests.cpp)
	target_link_libraries(geometry2_tests PRIVATE geometry2 GTest::GTest GTest::Main)
	add_test(NAME geometry2_tests COMMAND geometry2_tests)
endif()