#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <concepts>
#include <numbers>

#include "geometry2/math/matrix.hpp"
#include "geometry2/math/rotation.hpp"
#include "geometry2/primitives/coordinate_system.hpp"
#include "geometry2/transforms/transforms.hpp"

namespace {

using namespace geometry2;
namespace gmath = geometry2::math;

// Logical view image: +X right, +Y down; logical pixels (2 picture pixels per
// view pixel in this example).
struct View : CoordinateSystem<View, 2> {};

// Raster image: +X right, +Y down; pixels, origin at the top-left corner.
struct Picture : CoordinateSystem<Picture, 2> {};

// Optical camera frame: +X right, +Y down, +Z forward; metres for positions
// and displacements (directions are unitless).
struct Camera : CoordinateSystem<Camera, 3> {};

// UAV body FRB frame: +X forward, +Y right, +Z bottom/down; metres.
struct UavFrb : CoordinateSystem<UavFrb, 3> {};

// UAV local NED frame: +X north, +Y east, +Z down; metres.
struct UavNed : CoordinateSystem<UavNed, 3> {};

// Ground NED frame: +X north, +Y east, +Z down; metres, with a ground-fixed
// origin.
struct GndNed : CoordinateSystem<GndNed, 3> {};

using ViewPictureTransform = AffineTransform2<View, Picture>;
using PictureCameraTransform = PinholeProjectionTransform<Picture, Camera>;
using CameraUavFrbTransform = RotationTransform3<Camera, UavFrb>;
using UavFrbUavNedTransform = RotationTransform3<UavFrb, UavNed>;
using UavNedGndNedTransform = RigidTransform3<UavNed, GndNed>;

template <class Left, class Right>
concept Subtractable =
    requires(const Left& left, const Right& right) { left - right; };

template <class Value, class... Expected>
void expect_coordinates_near(const Value& actual,
                             Expected... expected_coordinates) {
    const std::array<double, sizeof...(Expected)> expected{
        static_cast<double>(expected_coordinates)...};
    for (std::size_t i = 0; i < expected.size(); ++i) {
        EXPECT_NEAR(actual[i], expected[i], 1e-9);
    }
}

auto view_to_picture_transform() {
    return ViewPictureTransform{gmath::SquareMatrix<2>{2.0, 0.0, 0.0, 2.0},
                                Picture::Vector{320.0, 240.0}};
}

auto picture_camera_projection() {
    return PictureCameraTransform{100.0, 100.0, 320.0, 240.0};
}

auto camera_to_uav_frb_transform() {
    return CameraUavFrbTransform{
        gmath::rotation_y(gmath::Radians{std::numbers::pi_v<double> / 2.0}) *
        gmath::rotation_z(gmath::Radians{std::numbers::pi_v<double> / 2.0})};
}

auto uav_frb_to_uav_ned_transform() {
    // The example UAV points north without roll or pitch, so FRD and NED axes
    // are aligned at this instant.
    return UavFrbUavNedTransform{gmath::identity_matrix<3, double>()};
}

auto uav_ned_to_ground_transform() {
    return UavNedGndNedTransform{gmath::identity_matrix<3, double>(),
                                 GndNed::Vector{10.0, 20.0, 30.0}};
}

auto coordinate_transforms() {
    return compose_transforms(
        view_to_picture_transform(), picture_camera_projection(),
        camera_to_uav_frb_transform(), uav_frb_to_uav_ned_transform(),
        uav_ned_to_ground_transform());
}

static_assert(
    std::same_as<decltype(View::Point{} - View::Point{}), View::Vector>);
static_assert(
    std::same_as<decltype(View::Point{} + View::Vector{}), View::Point>);
static_assert(
    std::same_as<decltype(View::Vector{} + View::Vector{}), View::Vector>);
static_assert(std::same_as<Point<View>, View::Point>);
static_assert(std::same_as<Vector<Camera>, Camera::Vector>);
static_assert(std::same_as<Point2<View>, View::Point>);
static_assert(std::same_as<Point3<Camera>, Camera::Point>);
static_assert(std::same_as<Vector2<View>, View::Vector>);
static_assert(std::same_as<Vector3<Camera>, Camera::Vector>);
static_assert(std::same_as<Dir3<Camera>, Camera::Dir>);
static_assert(std::same_as<Line3<GndNed>, GndNed::Line>);
static_assert(std::same_as<gmath::Point2<>, gmath::Point<2>>);
static_assert(std::same_as<gmath::Vector3<>, gmath::Vector<3>>);
static_assert(std::same_as<decltype(gmath::Point<2>{} - gmath::Point<2>{}),
                           gmath::Vector<2>>);
static_assert(!Subtractable<View::Point, Picture::Point>);
static_assert(gmath::MatrixLike<gmath::Matrix<2, 3>>);
static_assert(std::same_as<gmath::SquareMatrix<3>, gmath::Matrix<3, 3>>);

TEST(CoordinateMathTest,
     SupportsRectangularMatricesSeparatelyFromSquareMatrices) {
    const gmath::Matrix<2, 3> select_xy{1.0, 0.0, 0.0, 0.0, 1.0, 0.0};

    expect_coordinates_near(gmath::apply_matrix<Picture::Vector>(
                                select_xy, Camera::Vector{4.0, 5.0, 6.0}),
                            4.0, 5.0);
}

TEST(CoordinatePrimitivesTest,
     KeepsPointVectorAndFrameSemanticsInTheTypeSystem) {
    const View::Point first{1.0, 2.0};
    const View::Point second{4.0, 6.0};

    EXPECT_EQ(second - first, View::Vector(3.0, 4.0));
    EXPECT_EQ(first + View::Vector(3.0, 4.0), second);
    EXPECT_EQ(View::Vector(1.0, 2.0) + View::Vector(3.0, 4.0),
              View::Vector(4.0, 6.0));

    View::Point point2{1.0, 2.0};
    View::Vector vector2{3.0, 4.0};
    point2.x() = 5.0;
    vector2.dy() = 6.0;
    EXPECT_DOUBLE_EQ(point2.x(), 5.0);
    EXPECT_DOUBLE_EQ(point2.y(), 2.0);
    EXPECT_DOUBLE_EQ(vector2.dx(), 3.0);
    EXPECT_DOUBLE_EQ(vector2.dy(), 6.0);
}

TEST(CoordinatePrimitivesTest,
     StoresDirAndLineInCanonicalCalculationFriendlyForm) {
    const auto direction = dir_of(GndNed::Vector{2.0, 0.0, 0.0});
    expect_coordinates_near(direction, 1.0, 0.0, 0.0);
    EXPECT_NEAR(norm(as_vector(direction)), 1.0, 1e-9);
    expect_coordinates_near(dir_to(GndNed::Point{2.0, 0.0, 0.0}), 1.0, 0.0,
                            0.0);

    const auto line = line_through(GndNed::Point{10.0, 20.0, 30.0}, direction);

    // Moving the input anchor along the line would produce exactly the same
    // stored point: the closest one to the GndNed origin.
    expect_coordinates_near(closest_point_to_origin(line), 0.0, 20.0, 30.0);
    expect_coordinates_near(dir_of(line), 1.0, 0.0, 0.0);

    EXPECT_THROW(static_cast<void>(dir_of(GndNed::Vector{})),
                 std::invalid_argument);
}

TEST(GenericTransformsTest, AffineAndPinholeTransformsExposeOnlyValidKinds) {
    const auto view_picture = view_to_picture_transform();
    const auto projection = picture_camera_projection();

    const auto picture_point =
        view_picture(View::Point{5.0, -5.0}, Tag<Picture::Point>{});
    expect_coordinates_near(picture_point, 330.0, 230.0);
    expect_coordinates_near(view_picture(picture_point, Tag<View::Point>{}),
                            5.0, -5.0);

    const auto camera_direction = projection(picture_point, Tag<Camera::Dir>{});
    const double normalization = std::sqrt(1.02);
    expect_coordinates_near(camera_direction, 0.1 / normalization,
                            -0.1 / normalization, 1.0 / normalization);
    expect_coordinates_near(projection(camera_direction, Tag<Picture::Point>{}),
                            330.0, 230.0);
}

TEST(GenericTransformsTest, RigidTransformMovesPointsButNotVectors) {
    const auto rotation = camera_to_uav_frb_transform();
    const auto rigid = uav_ned_to_ground_transform();

    expect_coordinates_near(
        rotation(Camera::Point{1.0, 2.0, 3.0}, Tag<UavFrb::Point>{}), 3.0, 1.0,
        2.0);
    expect_coordinates_near(
        rigid(UavNed::Point{1.0, 2.0, 3.0}, Tag<GndNed::Point>{}), 11.0, 22.0,
        33.0);
    expect_coordinates_near(
        rigid(UavNed::Vector{1.0, 2.0, 3.0}, Tag<GndNed::Vector>{}), 1.0, 2.0,
        3.0);
}

TEST(CoordinateTransformGraphTest,
     ComposesConcreteCoordinateSystemsByGeometryKind) {
    const auto transforms = coordinate_transforms();

    // The center pixel becomes the camera +Z direction. The two rotations map
    // it to GndNed +X, while the rigid translation places the resulting line
    // through the UAV position (10, 20, 30).
    const auto ground_line = transforms.to<GndNed::Line>(View::Point{0.0, 0.0});
    expect_coordinates_near(dir_of(ground_line), 1.0, 0.0, 0.0);
    expect_coordinates_near(closest_point_to_origin(ground_line), 0.0, 20.0,
                            30.0);

    const auto ground_point =
        transforms.to<GndNed::Point>(Camera::Point{1.0, 2.0, 3.0});
    expect_coordinates_near(ground_point, 13.0, 21.0, 32.0);

    const auto ground_vector =
        transforms.to<GndNed::Vector>(Camera::Vector{1.0, 2.0, 3.0});
    expect_coordinates_near(ground_vector, 3.0, 1.0, 2.0);

    expect_coordinates_near(transforms.to<Camera::Point>(ground_point), 1.0,
                            2.0, 3.0);
    expect_coordinates_near(transforms.to<View::Point>(ground_point),
                            50.0 / 3.0, 100.0 / 3.0);
}

}  // namespace
