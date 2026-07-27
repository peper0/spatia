# spatia

Strongly typed geometry for C++23: a coordinate system is part of every geometric type, so invalid frame mixing is rejected while transformations compose naturally.

```cpp
struct Body  { static constexpr std::size_t dimension = 3; };
struct World { static constexpr std::size_t dimension = 3; };
using namespace spatia;
auto body_to_world = to_rotation(EulerZYX<Body, World>{
    Angle::from_degrees(30), Angle::from_degrees(5), Angle{}});

Point<Body> p_in_body{1, 0, 0};
Point<World> p_in_world = body_to_world.rotate_about_shared_origin(p_in_body);
Vector<World> v_in_world = p_in_world - Point<World>{0, 0, 0};
// p_in_world - Point<Body>{0, 0, 0}  // compile-time error: different systems
```

AI: dodaj tu jeszcze przykład combine na dwóch transformacjach z przekształceniem później do każdego z układów

## Start here

The [step-by-step guide](docs/guide.md) begins with two coordinate systems, then adds directions, domain angle names, rotation conversion, composition, quaternions, rigid transforms, and affine 2D transforms. A complete buildable project lives in [examples/guide](examples/guide).


## Build and test

The library is exposed as the CMake target `spatia::spatia`. Tests are enabled by default, while generated documentation is opt-in.

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

The standalone example can also use `examples/guide` as its CMake source directory.