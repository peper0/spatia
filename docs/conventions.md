See also [coding_style](./coding_style.md).

# Design conventions

## Transforms

A **transform** is a functor with one or more `operator()` overloads. A plain
function, a lambda, `Rotation`, `Rigid`, `PerspectiveProjection`, and the results
of `operator*` and `combine` are all transforms. A transform with more than
one overload additionally declares a `Transformations` typedef listing the
conversion edges it provides, because those cannot be inferred from a single
signature.

Two operations build new transforms out of existing ones:

- `bc * ab` takes two transforms and returns their composition: given
  `A -> B` and `B -> C` it yields `A -> C`. Following the matrix convention,
  the right operand is applied first. When both operands are the same kind of
  geometric transform, the result is a single object of that kind (two
  `Rotation`s collapse into one `Rotation`); when both provide exactly one
  conversion, the result chains the two calls.
- `combine(...)` takes any number of transforms and returns one transform
  providing all of their conversions plus every conversion reachable by
  chaining them. Conversions are requested by destination type, either as
  `to<Destination>(object)` or `transform(object, Tag<Destination>{})`.

Each conversion is a single `operator()` overload taking the destination tag
as a defaulted argument, so `transform(object)` and
`transform(object, Tag<Destination>{})` are the same function. The tag is
spelled out only where one source type has several destinations and the plain
call would be ambiguous, or where naming the destination is the point:

- `Rotation` maps a `Point` only through `rotate_about_shared_origin` or an
  explicit `Tag<Point<To>>`, because rotating a point assumes the two systems
  share an origin.
- `Rigid` maps a `Dir` to a `Dir` by default; producing the line through the
  transformed origin instead needs `Tag<Line<To>>`.

## Naming and conversions

- A transform assembled from smaller transforms takes them as such: `Rigid`
  is built from a `Rotation` and a `Translation`. A second constructor takes
  the raw algebra instead (`Matrix` and `Vec`), so every constructor is either
  fully typed or fully raw.
- When a type has factory functions, they are named `from_...`
  (`Angle::from_degrees`, `Dir::from_vector`).
- Free conversion functions are named `to_...` (`to_rotation`, `to_euler_zyx`,
  `to_vector`).
- Whenever possible a conversion is a free function, so the source type does
  not have to know every representation it can convert to. When a member
  implementation is more efficient, the same function may additionally be
  provided as a method (`Rotation::to_matrix`, `Rotation::to_quaternion`).

# Mathematical conventions and decisions

This page is the precise contract for representation conversions.


`CoordinateSystem<Derived, Dimension>` is an optional facade that additionally provides `Derived::Point`, `Derived::Vector`, `Derived::Dir`, and `Derived::Line` aliases. The scalar type is not configurable.

## Rotation convention

Rotations are active, right-handed rotations acting on column vectors. `Rotation<From, To>` stores the matrix that maps coordinates expressed in `From` to coordinates expressed in `To`. It transforms in that direction only; `inverse()` returns the opposite direction, and `BiRotation<A, B>` derives from both `Rotation<A, B>` and `Rotation<B, A>` so one object serves both directions (the same pattern exists for rigid, affine, and translation transforms).

The direct matrix constructor has a precondition that its argument is a proper orthonormal rotation matrix. It deliberately does not repeat that validation at runtime; conversions from Euler angles and normalized quaternions satisfy the invariant by construction.

For compatible transforms `ab: A -> B` and `bc: B -> C`:

```text
ac = bc * ab
v_c = bc(ab(v_a))
```

## Euler Z-Y-X

`EulerZYX<From, To>{z, y, x}` uses intrinsic Z-Y-prime-X-double-prime Tait-Bryan angles. The equivalent matrix and extrinsic description are:

```text
R = Rz(z) * Ry(y) * Rx(x)
extrinsic X-Y-Z, with X applied first
```

Positive angles follow the right-hand rule. Conversion from a matrix returns:

- `y` in `[-pi/2, pi/2]`;
- `z` and `x` in `[-pi, pi]` away from numerical boundary equivalences;
- at gimbal lock, `x = 0` and `z` stores the remaining observable rotation.

## Euler Z-Y directions and NED

`EulerZY<System>{z, y}` parameterizes the direction obtained by applying `Rz(z) * Ry(y)` to the positive X axis:

```text
dir = (cos(z) cos(y), sin(z) cos(y), -sin(y))
```

For a conventional NED system `(X north, Y east, Z down)`, positive `z` turns north toward east and positive `y` points above the horizontal plane because it produces negative Z. This makes a domain facade named `AziElev` read naturally: positive elevation means up.

`to_euler_zy` returns `y` in `[-pi/2, pi/2]` and `z` in `[-pi, pi]`. At either vertical pole azimuth is undefined, so the canonical result chooses `z = 0`.

## Angle

`Angle` stores radians internally, but this is not exposed through an ambiguous scalar constructor. `from_degrees` and `from_radians` label input units. Arithmetic does not normalize automatically.

- `normalized_unsigned()` returns `[0, 2pi)`.
- `normalized_signed()` returns `[-pi, pi)`.

## Quaternion

`Quaternion` is untagged algebra. Components and constructor arguments are scalar-first `(w, x, y, z)`. Multiplication is the Hamilton product. A unit quaternion acts as:

```text
v_rotated = q * (0, v) * conjugate(q)
```

Therefore `q_left * q_right` applies `q_right` first. Both `q` and `-q` represent the same rotation. Matrix-to-quaternion conversion normalizes the result and chooses the sign for which the first non-zero component in `(w, x, y, z)` is positive.

## Direction representation

`Dir<System>` stores a normalized Cartesian vector, not two angles. One component is mathematically redundant, but the representation has no angular seam, applies a matrix directly, and requires no trigonometry during ordinary transformations. Construction normalizes and rejects a zero vector. The coordinates are read through `to_vector(direction)`, so the invariant cannot be broken.

## Rotation and points

`Rotation` has `operator()` for `Vector` and `Dir`. It intentionally does not provide ordinary call syntax for `Point`, because a rotation alone does not encode the relationship between coordinate-system origins. When the caller knows the origins coincide, `rotate_about_shared_origin(point)` makes that assumption explicit. General points use `Rigid` or `Affine`.

## Transform hierarchy

`Rotation`, `Rigid`, `Affine`, and `Projective` are separate value types with progressively weaker invariants. There is no common base class: this avoids virtual dispatch and keeps composition results concrete.

Multiplying two transforms yields the narrowest kind that can represent the result, so `Rotation * Rotation` is a `Rotation` while `Rotation * Affine` is an `Affine`. `Projective` is the closure of the family: it stores a homogeneous matrix with one extra row and column, so it also covers `PerspectiveProjection`, which is projective but not affine. Any product involving a projective operand is projective.

Going the other way is a **narrowing conversion**, and it can fail: `to_rigid(affine, tolerance)`, `to_rotation(rigid, tolerance)`, and `to_translation(rigid, tolerance)` check that the argument already satisfies the narrower invariant within `tolerance` (an absolute error on matrix entries and coordinates, `default_narrowing_tolerance` by default) and throw `std::invalid_argument` otherwise. The widening direction — `to_affine`, `to_rigid` from a rotation or translation, `to_projective` — always succeeds and takes no tolerance.

## Projection direction

`PerspectiveProjection<FromSpace, ToPlane>` projects points of a 3D system onto a 2D surface; `PerspectiveUnprojection<FromPlane, ToSpace>` goes back and yields a `Dir`, because the distance along the projection axis is not recoverable. `BiPerspectiveProjection<Space, Plane>` derives from both, following the same pattern as the other `Bi*` transforms.
