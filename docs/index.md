# spatia

`spatia` makes coordinate systems part of C++ types. A vector in a camera system cannot accidentally be added to a vector in a world system, while a typed transformation performs the conversion explicitly.

```cpp
struct Camera { static constexpr std::size_t dimension = 3; };
struct World  { static constexpr std::size_t dimension = 3; };

Rotation<Camera, World> camera_to_world = to_rotation(
    EulerZYX<Camera, World>{Angle::from_degrees(20), Angle{}, Angle{}});
Vector<World> ray = camera_to_world(Vector<Camera>{0, 0, 1});
```

The library is split into algebra, geometry, and transformation layers without splitting the public namespace. Start with the guide, then use the conventions page as the precise contract.

The <a href="api/index.html">API reference</a> is generated from the public
headers by Doxygen. It is written next to these pages after Sphinx runs, so
Sphinx cannot check the link itself — hence the raw anchor.

```{toctree}
:maxdepth: 2
:caption: Contents

guide
conventions
coding_style
```
