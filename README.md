# spatia

Geometry for C++23 that keeps track of coordinate systems

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

Hand `combine` the transforms you happen to have, and you can then ask for any
system you like — it works out the route, in either direction, at compile time:

```cpp
struct Camera { static constexpr std::size_t dimension = 3; };

auto camera_to_body = to_rotation(EulerZYX<Camera, Body>{
    Angle::from_degrees(90), Angle{}, Angle{}});

auto transforms = combine(
    BiRigid<Camera, Body>{camera_to_body,
                          Translation<Camera, Body>{Vector<Body>{0.1, 0.0, -0.2}}},
    BiRigid<Body, World>{body_to_world,
                         Translation<Body, World>{Vector<World>{10.0, 20.0, 30.0}}});

Point<Camera> target{0.0, 0.0, 5.0};

Point<Body>   in_body  = transforms.to<Point<Body>>(target);
Point<World>  in_world = transforms.to<Point<World>>(target);
Point<Camera> back     = transforms.to<Point<Camera>>(in_world);
```

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