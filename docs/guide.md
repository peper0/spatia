# Step-by-step guide

All examples use the umbrella header and the single public namespace:

```cpp
#include <spatia/spatia.hpp>
using namespace spatia;
```

## 1. Define two coordinate systems

A coordinate system is only a type with a compile-time dimension. It carries no runtime state.

```cpp
struct Body {
    static constexpr std::size_t dimension = 3;
};

struct World {
    static constexpr std::size_t dimension = 3;
};

Vector<Body> body_forward{1.0, 0.0, 0.0};
Point<World> landmark{10.0, 20.0, 2.0};
```

The system controls the dimension and participates in the type:

```cpp
static_assert(dimension_v<Body> == 3);
// body_forward + Vector<World>{};  // error: different systems
```

## 2. Create a rotation between them

Angles never accept an unlabelled scalar. Use degrees or radians explicitly. `EulerZYX<From, To>` also carries the direction of the transformation.

```cpp
EulerZYX<Body, World> attitude{
    Angle::from_degrees(30.0),  // Z: yaw
    Angle::from_degrees(5.0),   // Y: pitch
    Angle::from_degrees(1.0)    // X: roll
};

Rotation<Body, World> body_to_world = to_rotation(attitude);
Vector<World> world_forward = body_to_world(body_forward);
```

The result cannot be assigned to `Vector<Body>`. The transform is the explicit proof that the system changed.

## 3. Invert and multiply

`inverse()` swaps the system order. Composition is multiplication and follows matrix notation: the right operand is applied first.

```cpp
Rotation<World, Body> world_to_body = body_to_world.inverse();
Vector<Body> restored = world_to_body(world_forward);

struct Camera { static constexpr std::size_t dimension = 3; };
Rotation<Camera, Body> camera_to_body = to_rotation(
    EulerZYX<Camera, Body>{Angle::from_degrees(2), Angle{}, Angle{}});

Rotation<Camera, World> camera_to_world = body_to_world * camera_to_body;
```

The camera-to-body rotation runs first, and composing two rotations yields a
single `Rotation` rather than a wrapper.

## 4. Parameterize a direction with two angles

`EulerZY<System>` uses Z for azimuth-like rotation and Y for elevation-like rotation. It converts through a separate adapter header to keep the underlying types independent.

```cpp
EulerZY<World> look_angles{
    Angle::from_degrees(45.0),
    Angle::from_degrees(10.0)
};

Dir<World> look = to_dir(look_angles);
EulerZY<World> restored_angles = to_euler_zy(look);
```

`Dir` stores normalized Cartesian components. This is redundant by one scalar but makes application of matrices cheap and avoids angular seams during ordinary calculations. The coordinates are read through `to_vector(direction)`, so the unit-length invariant cannot be broken.

## 5. Add domain vocabulary without changing representation

Applications can derive a light facade and rename neutral axes. A non-explicit base-value constructor is appropriate when it changes no signs, order, ranges, or validation.

```cpp
class AziElev final : public EulerZY<World> {
    using Base = EulerZY<World>;
public:
    AziElev(Base value) : Base{std::move(value)} {}
    Angle& azimuth() noexcept { return z(); }
    Angle const& azimuth() const noexcept { return z(); }
    Angle& elevation() noexcept { return y(); }
    Angle const& elevation() const noexcept { return y(); }
};

AziElev named = to_euler_zy(look);
```

Use composition and an explicit conversion instead if a domain type changes semantics.

## 6. Convert rotation representations

The typed `Rotation` owns the `From -> To` relation. A quaternion is intentionally algebraic and untagged.

```cpp
Quaternion q = to_quaternion(body_to_world);
Rotation<Body, World> from_q = to_rotation<Body, World>(q);
EulerZYX<Body, World> restored_attitude = to_euler_zyx(from_q);
```

The explicit template arguments on quaternion-to-rotation conversion attach the missing systems.

## 7. Move points with a rigid transform

A bare rotation naturally transforms vectors and directions. A rigid transform also supplies the relationship between origins.

```cpp
Rigid<Body, World> pose{
    body_to_world,
    Translation<Body, World>{Vector<World>{100.0, 50.0, -10.0}}
};

Point<World> sensor = pose(Point<Body>{1.0, 0.0, 0.0});
Vector<World> velocity = pose(Vector<Body>{5.0, 0.0, 0.0});
```

Translation affects the point but not the vector. If two systems deliberately share an origin, a rotation can transform a point through the explicit `rotate_about_shared_origin()` member.

## 8. Work in 2D and cross the algebra boundary

```cpp
struct Image  { static constexpr std::size_t dimension = 2; };
struct Screen { static constexpr std::size_t dimension = 2; };

Affine<Image, Screen> image_to_screen{
    Matrix<2, 2>{2.0, 0.0, 0.0, 2.0},
    Vector<Screen>{320.0, 240.0}
};
Point<Screen> pixel = image_to_screen(Point<Image>{10.0, 20.0});

Vec<2> raw{3.0, 4.0};
Vector<Image> offset = Vector<Image>::from_vec(raw);
Vec<2> exported = offset.to_vec();
```

The `from_vec` and `to_vec` calls make loss or attachment of coordinate-system semantics visible in code.

A rotation in the plane is a two-dimensional `Rotation` parameterized by a
single angle:

```cpp
Rotation<Image, Screen> upright = to_rotation<Image, Screen>(
    Angle::from_degrees(90.0));
Angle recovered = to_angle(upright);
```

## 9. Combine many transforms into a graph

Everything callable that converts objects is a *transform*: a lambda, a
`Rotation`, a `PerspectiveProjection`. `combine(...)` accepts any number of them
and returns one transform providing all of their conversions plus every
conversion reachable by chaining them. `to<Destination>()` finds a shortest
path of edges from the argument's type to the destination at compile time and
applies the transforms along it. A missing path is a compile-time error.

```cpp
struct View    { static constexpr std::size_t dimension = 2; };
struct Picture { static constexpr std::size_t dimension = 2; };
struct Camera  { static constexpr std::size_t dimension = 3; };
struct World   { static constexpr std::size_t dimension = 3; };

auto transforms = combine(
    BiAffine<View, Picture>{Matrix<2, 2>{2.0, 0.0, 0.0, 2.0},
                            Vector<Picture>{320.0, 240.0}},
    BiPerspectiveProjection<Camera, Picture>{100.0, 100.0, 320.0, 240.0},
    BiRigid<Camera, World>{camera_to_world_rotation,
                           Translation<Camera, World>{Vector<World>{10.0, 20.0, 30.0}}});

// View pixel -> Picture pixel -> Camera direction -> World line:
Line<World> ray = transforms.to<Line<World>>(Point<View>{0.0, 0.0});

// The reverse direction traverses the Bi* edges:
Point<View> back = transforms.to<Point<View>>(world_point);
```

Three properties are worth remembering:

- One-directional transforms contribute only their `From -> To` edges. Use the
  `Bi*` variants (`BiRotation`, `BiRigid`, `BiAffine`,
  `BiTranslation`) when the graph should traverse a transform both
  ways.
- Rotations move points through the graph via the shared-origin
  interpretation, exactly as `rotate_about_shared_origin()` would outside it.
- The search minimizes the number of edges, not any notion of accuracy: if
  you add custom edges that lose information (like a projection), a short
  lossy path wins over a long exact one.

`bc * ab` is the two-operand counterpart: it returns the single composed
`A -> C` transform. For two transforms of the same geometric kind that is one
object of that kind (two `Rotation`s give a `Rotation<A, C>`); for transforms
that provide exactly one conversion each, it chains the two calls:

```cpp
const PerspectiveUnprojection<Picture, Camera> projection{100.0, 100.0, 320.0, 240.0};

const auto view_to_picture = [](const Point<View>& p) {
    return Point<Picture>{p.x() * 2.0, p.y() * 2.0};
};
const auto picture_to_ray = [&](const Point<Picture>& p) {
    return projection(p, Tag<Dir<Camera>>{});
};

auto view_to_ray = picture_to_ray * view_to_picture;
Dir<Camera> ray = view_to_ray(Point<View>{0.0, 0.0});
```

A transform that provides several conversions (like `PerspectiveProjection` or any
`Bi*` type) cannot be composed this way, because the intermediate conversion
would be ambiguous. Either pick one conversion with a `Tag`, as the lambda
above does, or use `combine`.

## Complete project

[`examples/guide`](https://github.com/peper0/spatia/tree/main/examples/guide) is a standalone CMake project containing the four systems from the specification, domain angle facades, direction conversions, rotations, composition, inversion, a quaternion, a rigid transform, 2D types, and commented compile-time failures.
