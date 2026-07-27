#!/usr/bin/env bash

set -euo pipefail

script_directory="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repository_root="$(cd -- "${script_directory}/.." && pwd)"
build_directory="${repository_root}/build/compile-time"
compile_timeout_seconds="${GEOMETRY2_BENCHMARK_TIMEOUT_SECONDS:-30}"
memory_limit_mib="${GEOMETRY2_BENCHMARK_MEMORY_LIMIT_MIB:-10240}"

if [[ ! "${compile_timeout_seconds}" =~ ^[1-9][0-9]*$ ]]; then
    echo "GEOMETRY2_BENCHMARK_TIMEOUT_SECONDS must be a positive integer." >&2
    exit 2
fi

if [[ ! "${memory_limit_mib}" =~ ^[1-9][0-9]*$ ]]; then
    echo "GEOMETRY2_BENCHMARK_MEMORY_LIMIT_MIB must be a positive integer." >&2
    exit 2
fi

cmake \
    -S "${repository_root}" \
    -B "${build_directory}" \
    -DBUILD_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release

# This directory is dedicated to the benchmark. Cleaning it makes repeated
# measurements comparable without touching regular build directories.
cmake --build "${build_directory}" --target clean --parallel 1

shopt -s nullglob
variant_headers=(
    "${repository_root}"/benchmarks/compose_variants/*/compose.hpp
)
scenarios=(large_graph many_uses four_trees_unrolled four_trees_generated)

if (( ${#variant_headers[@]} == 0 )); then
    echo "No compose variants found." >&2
    exit 1
fi

failed_benchmarks=()
result_variants=()
declare -A result_times
declare -A result_memory_mib

printf "Benchmark limits: %s s and %s MiB per build.\n" \
    "${compile_timeout_seconds}" "${memory_limit_mib}"

for variant_header in "${variant_headers[@]}"; do
    variant_directory="${variant_header%/*}"
    variant_name="${variant_directory##*/}"
    result_variants+=("${variant_name}")

    for scenario in "${scenarios[@]}"; do
        target="spatia_compile_time_tests_${variant_name}_${scenario}"
        metrics_file="${build_directory}/compile_time_metrics/${variant_name}_${scenario}.csv"
        build_log="${build_directory}/compile_time_metrics/${variant_name}_${scenario}.log"
        result_key="${variant_name}:${scenario}"

        : > "${metrics_file}"
        : > "${build_log}"
        printf "\n=== %s / %s ===\n" "${variant_name}" "${scenario}"

        if (
            ulimit -Sv "$((memory_limit_mib * 1024))"
            timeout \
                --signal=TERM \
                --kill-after=5s \
                "${compile_timeout_seconds}s" \
                cmake \
                    --build "${build_directory}" \
                    --target "${target}" \
                    --parallel 1
        ) > "${build_log}" 2>&1; then
            build_status="OK"
        else
            exit_code=$?
            if (( exit_code == 124 || exit_code == 137 )); then
                build_status="TIMEOUT"
            else
                build_status="FAILED"
            fi
            failed_benchmarks+=("${variant_name}/${scenario}:${build_status}")
        fi

        result_times["${result_key}"]="${build_status}"
        result_memory_mib["${result_key}"]="-"
        elapsed_seconds="-"
        max_rss_mib="-"

        if [[ -s "${metrics_file}" ]]; then
            IFS=, read -r elapsed_seconds max_rss_kib < "${metrics_file}"

            if [[ "${build_status}" == "OK" ]]; then
                result_times["${result_key}"]="${elapsed_seconds}"
            fi

            if [[ "${max_rss_kib}" =~ ^[0-9]+$ ]]; then
                max_rss_mib="$(
                    awk -v kib="${max_rss_kib}" "BEGIN { printf \"%.1f\", kib / 1024 }"
                )"
                result_memory_mib["${result_key}"]="${max_rss_mib}"
            fi
        fi

        printf "result: %s | time [s]: %s | max RSS [MiB]: %s\n" \
            "${build_status}" "${elapsed_seconds}" "${max_rss_mib}"
        if [[ "${build_status}" != "OK" ]]; then
            printf "log: %s\n" "${build_log}"
        fi
    done
done

printf "\n%-24s | %15s | %15s | %17s | %18s | %19s | %20s | %20s | %21s\n" \
    "variant" \
    "20x20 time [s]" \
    "20x20 RSS [MiB]" \
    "8x8 uses time [s]" \
    "8x8 uses RSS [MiB]" \
    "unrolled time [s]" \
    "unrolled RSS [MiB]" \
    "generated time [s]" \
    "generated RSS [MiB]"
printf "%s\n" "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"

for variant_name in "${result_variants[@]}"; do
    large_key="${variant_name}:large_graph"
    uses_key="${variant_name}:many_uses"
    unrolled_key="${variant_name}:four_trees_unrolled"
    generated_key="${variant_name}:four_trees_generated"
    printf "%-24s | %15s | %15s | %17s | %18s | %19s | %20s | %20s | %21s\n" \
        "${variant_name}" \
        "${result_times[${large_key}]}" \
        "${result_memory_mib[${large_key}]}" \
        "${result_times[${uses_key}]}" \
        "${result_memory_mib[${uses_key}]}" \
        "${result_times[${unrolled_key}]}" \
        "${result_memory_mib[${unrolled_key}]}" \
        "${result_times[${generated_key}]}" \
        "${result_memory_mib[${generated_key}]}"
done

if (( ${#failed_benchmarks[@]} != 0 )); then
    printf "\nFailed benchmarks: %s\n" "${failed_benchmarks[*]}" >&2
    exit 1
fi

printf "\nAll compose variants compiled successfully.\n"
