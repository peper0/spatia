# Compose variants

Each subdirectory is a self-contained implementation of the same `compose.hpp`
interface: hand it a set of transforms and it resolves, at compile time, a
conversion from any source type to any destination type reachable through them.

`variant_a`, `variant_b`, and `variant_c` are three independent designs for
that search. Each has an `_opt` counterpart, which is the same design reworked
for compile-time cost after the first measurements.

The variants are not part of the library. They are built only by the
compile-time benchmark, which compiles every one of them against every scenario
in `tests/compile_time` and reports build time and peak memory:

```text
./scripts/benchmark_compile_times.sh
```

They are kept in the repository because they are the evidence behind the
implementation the library actually ships.
