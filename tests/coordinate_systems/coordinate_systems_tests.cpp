#include <gtest/gtest.h>

#include <cmath>
#include <concepts>
#include <numbers>

#include "spatia/algebra/matrix.hpp"
#include "spatia/geometry/coordinate_system.hpp"
#include "spatia/geometry/line_utils.hpp"
#include "spatia/testing/matchers.hpp"
#include "spatia/transforms.hpp"

namespace {

using namespace spatia;

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

using ViewPictureTransform = BiAffine<View, Picture>;
using PictureCameraTransform = BiPerspectiveProjection<Camera, Picture>;
using CameraUavFrbTransform = BiRotation<Camera, UavFrb>;
using UavFrbUavNedTransform = BiRotation<UavFrb, UavNed>;
using UavNedGndNedTransform = BiRigid<UavNed, GndNed>;

template <class Left, class Right>
concept Subtractable = requires(const Left& left, const Right& right) { left - right; };

auto view_to_picture_transform() {
    return ViewPictureTransform{SquareMatrix<2>{2.0, 0.0, 0.0, 2.0}, Picture::Vector{320.0, 240.0}};
}

auto picture_camera_projection() { return PictureCameraTransform{100.0, 100.0, 320.0, 240.0}; }

auto camera_to_uav_frb_transform() {
    // Maps camera (x right, y down, z forward) to FRB
    // (x forward, y right, z down): (x, y, z) -> (z, x, y).
    return CameraUavFrbTransform{Matrix<3, 3>{0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0}};
}

auto uav_frb_to_uav_ned_transform() {
    // The example UAV points north without roll or pitch, so FRD and NED axes
    // are aligned at this instant.
    return UavFrbUavNedTransform{identity_matrix<3>()};
}

auto uav_ned_to_ground_transform() {
    return UavNedGndNedTransform{identity_matrix<3>(), Point<GndNed>{10.0, 20.0, 30.0}};
}

auto coordinate_transforms() {
    return combine(view_to_picture_transform(), picture_camera_projection(), camera_to_uav_frb_transform(),
                   uav_frb_to_uav_ned_transform(), uav_ned_to_ground_transform());
}

static_assert(std::same_as<decltype(View::Point{} - View::Point{}), View::Vector>);
static_assert(std::same_as<decltype(View::Point{} + View::Vector{}), View::Point>);
static_assert(std::same_as<decltype(View::Vector{} + View::Vector{}), View::Vector>);
static_assert(std::same_as<Point<View>, View::Point>);
static_assert(std::same_as<Vector<Camera>, Camera::Vector>);
static_assert(std::same_as<decltype(Vec<2>{} + Vec<2>{}), Vec<2>>);
static_assert(!Subtractable<View::Point, Picture::Point>);
static_assert(std::same_as<SquareMatrix<3>, Matrix<3, 3>>);

TEST(CoordinateMathTest, SupportsRectangularMatricesSeparatelyFromSquareMatrices) {
    const Matrix<2, 3> select_xy{1.0, 0.0, 0.0, 0.0, 1.0, 0.0};

    EXPECT_THAT((Picture::Vector{select_xy * Camera::Vector{4.0, 5.0, 6.0}.to_vec()}),
                coordinates_near(Picture::Vector{4.0, 5.0}, 1e-9));
}

TEST(CoordinatePrimitivesTest, KeepsPointVectorAndFrameSemanticsInTheTypeSystem) {
    const View::Point first{1.0, 2.0};
    const View::Point second{4.0, 6.0};

    EXPECT_EQ(second - first, View::Vector(3.0, 4.0));
    EXPECT_EQ(first + View::Vector(3.0, 4.0), second);
    EXPECT_EQ(View::Vector(1.0, 2.0) + View::Vector(3.0, 4.0), View::Vector(4.0, 6.0));

    View::Point point2{1.0, 2.0};
    View::Vector vector2{3.0, 4.0};
    point2.x() = 5.0;
    vector2.dy() = 6.0;
    EXPECT_DOUBLE_EQ(point2.x(), 5.0);
    EXPECT_DOUBLE_EQ(point2.y(), 2.0);
    EXPECT_DOUBLE_EQ(vector2.dx(), 3.0);
    EXPECT_DOUBLE_EQ(vector2.dy(), 6.0);
}

TEST(CoordinatePrimitivesTest, StoresDirAndLineInCanonicalCalculationFriendlyForm) {
    const auto direction = dir_of(GndNed::Vector{2.0, 0.0, 0.0});
    EXPECT_THAT(to_vector(direction), coordinates_near(GndNed::Vector{1.0, 0.0, 0.0}, 1e-9));
    EXPECT_NEAR(norm(to_vector(direction)), 1.0, 1e-9);
    EXPECT_THAT(to_vector(dir_to(GndNed::Point{2.0, 0.0, 0.0})), coordinates_near(GndNed::Vector{1.0, 0.0, 0.0}, 1e-9));

    const auto line = line_through(GndNed::Point{10.0, 20.0, 30.0}, direction);

    // Moving the input anchor along the line would produce exactly the same
    // stored point: the closest one to the GndNed origin.
    EXPECT_THAT(closest_point_to_origin(line), coordinates_near(GndNed::Point{0.0, 20.0, 30.0}, 1e-9));
    EXPECT_THAT(to_vector(dir_of(line)), coordinates_near(GndNed::Vector{1.0, 0.0, 0.0}, 1e-9));

    EXPECT_THROW(static_cast<void>(dir_of(GndNed::Vector{})), std::invalid_argument);
}

TEST(CoordinatePrimitivesTest, ComparesAndMeasuresLinesGeometrically) {
    const auto direction = dir_of(GndNed::Vector{1.0, 1.0, 0.0});
    const auto line = line_through(GndNed::Point{1.0, 2.0, 3.0}, direction);
    const auto same_line = line_through(GndNed::Point{2.0, 3.0, 3.0}, direction);
    EXPECT_EQ(line, same_line);

    const auto shifted = line_through(GndNed::Point{1.0, 2.0, 7.5}, direction);
    EXPECT_NE(line, shifted);
    EXPECT_TRUE(are_parallel(line, shifted));
    EXPECT_NEAR(distance(line, shifted), 4.5, 1e-9);
    EXPECT_NEAR(distance(line, same_line), 0.0, 1e-9);

    const auto crossing = line_through(GndNed::Point{0.0, 0.0, 1.0}, dir_of(GndNed::Vector{1.0, -1.0, 0.0}));
    EXPECT_FALSE(are_parallel(line, crossing));
    EXPECT_NEAR(distance(line, crossing), 2.0, 1e-9);
}

TEST(GenericTransformsTest, AffineAndPerspectiveTransformsExposeOnlyValidKinds) {
    const auto view_picture = view_to_picture_transform();
    const auto projection = picture_camera_projection();

    const auto picture_point = view_picture(View::Point{5.0, -5.0}, Tag<Picture::Point>{});
    EXPECT_THAT(picture_point, coordinates_near(Picture::Point{330.0, 230.0}, 1e-9));
    EXPECT_THAT(view_picture(picture_point, Tag<View::Point>{}), coordinates_near(View::Point{5.0, -5.0}, 1e-9));

    const auto camera_direction = projection(picture_point, Tag<Camera::Dir>{});
    const double normalization = std::sqrt(1.02);
    EXPECT_THAT(to_vector(camera_direction), coordinates_near(Camera::Vector{0.1 / normalization, -0.1 / normalization, 1.0 / normalization}, 1e-9));
    EXPECT_THAT(projection(camera_direction, Tag<Picture::Point>{}), coordinates_near(Picture::Point{330.0, 230.0}, 1e-9));
}

TEST(GenericTransformsTest, ComposesSingleConversionTransformsIntoOne) {
    const auto projection = picture_camera_projection();

    const auto view_to_picture = [](const View::Point& p) { return Picture::Point{p.x() * 2.0, p.y() * 2.0}; };
    const auto picture_to_ray = [&](const Picture::Point& p) { return projection(p, Tag<Camera::Dir>{}); };

    const auto view_to_ray = picture_to_ray * view_to_picture;

    // The view centre maps to picture (0, 0), which is up and to the left of
    // the principal point (320, 240), so the ray points that way in camera
    // coordinates.
    const Camera::Dir ray = view_to_ray(View::Point{0.0, 0.0});
    const auto expected = projection(Picture::Point{0.0, 0.0}, Tag<Camera::Dir>{});
    EXPECT_THAT(to_vector(ray), coordinates_near(to_vector(expected), 1e-9));
}

TEST(GenericTransformsTest, RigidMovesPointsButNotVectors) {
    const auto rotation = camera_to_uav_frb_transform();
    const auto rigid = uav_ned_to_ground_transform();

    EXPECT_THAT(rotation(Camera::Point{1.0, 2.0, 3.0}), coordinates_near(UavFrb::Point{3.0, 1.0, 2.0}, 1e-9));
    EXPECT_THAT(rigid(UavNed::Point{1.0, 2.0, 3.0}, Tag<GndNed::Point>{}), coordinates_near(GndNed::Point{11.0, 22.0, 33.0}, 1e-9));
    EXPECT_THAT(rigid(UavNed::Vector{1.0, 2.0, 3.0}, Tag<GndNed::Vector>{}), coordinates_near(GndNed::Vector{1.0, 2.0, 3.0}, 1e-9));
}

TEST(CoordinateTransformGraphTest, ComposesConcreteCoordinateSystemsByGeometryKind) {
    const auto transforms = coordinate_transforms();

    // The center pixel becomes the camera +Z direction. The two rotations map
    // it to GndNed +X, while the rigid translation places the resulting line
    // through the UAV position (10, 20, 30).
    const auto ground_line = transforms.to<GndNed::Line>(View::Point{0.0, 0.0});
    EXPECT_THAT(to_vector(dir_of(ground_line)), coordinates_near(GndNed::Vector{1.0, 0.0, 0.0}, 1e-9));
    EXPECT_THAT(closest_point_to_origin(ground_line), coordinates_near(GndNed::Point{0.0, 20.0, 30.0}, 1e-9));

    const auto ground_point = transforms.to<GndNed::Point>(Camera::Point{1.0, 2.0, 3.0});
    EXPECT_THAT(ground_point, coordinates_near(GndNed::Point{13.0, 21.0, 32.0}, 1e-9));

    const auto ground_vector = transforms.to<GndNed::Vector>(Camera::Vector{1.0, 2.0, 3.0});
    EXPECT_THAT(ground_vector, coordinates_near(GndNed::Vector{3.0, 1.0, 2.0}, 1e-9));

    EXPECT_THAT(transforms.to<Camera::Point>(ground_point), coordinates_near(Camera::Point{1.0, 2.0, 3.0}, 1e-9));
    EXPECT_THAT(transforms.to<View::Point>(ground_point), coordinates_near(View::Point{50.0 / 3.0, 100.0 / 3.0}, 1e-9));
}

}  // namespace
