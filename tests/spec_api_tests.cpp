#include <gtest/gtest.h>

#include <cmath>
#include <concepts>
#include <numbers>
#include <type_traits>
#include <utility>

#include "spatia/spatia.hpp"

namespace {

using namespace spatia;

struct Ground {
    static constexpr std::size_t dimension = 3;
};

struct Body {
    static constexpr std::size_t dimension = 3;
};

struct Camera {
    static constexpr std::size_t dimension = 3;
};

struct Image {
    static constexpr std::size_t dimension = 2;
};

struct Screen {
    static constexpr std::size_t dimension = 2;
};

class AziElev final : public EulerZY<Ground> {
    using Base = EulerZY<Ground>;

   public:
    AziElev(Base value) : Base{std::move(value)} {}

    Angle& azimuth() noexcept { return z(); }
    const Angle& azimuth() const noexcept { return z(); }
    Angle& elevation() noexcept { return y(); }
    const Angle& elevation() const noexcept { return y(); }
};

class YawPitchRoll final : public EulerZYX<Body, Ground> {
    using Base = EulerZYX<Body, Ground>;

   public:
    YawPitchRoll(Base value) : Base{std::move(value)} {}

    Angle& yaw() noexcept { return z(); }
    const Angle& yaw() const noexcept { return z(); }
    Angle& pitch() noexcept { return y(); }
    const Angle& pitch() const noexcept { return y(); }
    Angle& roll() noexcept { return x(); }
    const Angle& roll() const noexcept { return x(); }

    static YawPitchRoll from_transform(const Rotation<Body, Ground>& transform) { return to_euler_zyx(transform); }
};

template <class Left, class Right>
concept Addable = requires(Left left, Right right) { left + right; };

static_assert(std::same_as<Scalar, double>);
static_assert(dimension_v<Ground> == 3);
static_assert(dimension_v<Image> == 2);
static_assert(std::same_as<decltype(Point<Ground>{} - Point<Ground>{}), Vector<Ground>>);
static_assert(!Addable<Vector<Ground>, Vector<Body>>);
static_assert(!std::invocable<Rotation<Body, Ground>, Vector<Camera>>);
static_assert(std::same_as<decltype(std::declval<Rotation<Body, Ground>>() * std::declval<Rotation<Camera, Body>>()),
                           Rotation<Camera, Ground>>);

TEST(SpecAlgebraTest, SeparatesRawCoordinatesFromGeometry) {
    const Vec<3> coordinates{1.0, 2.0, 3.0};
    const Matrix<2, 3> select_xy{1.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    const Vec<2> xy = select_xy * coordinates;

    EXPECT_EQ(xy, (Vec<2>{1.0, 2.0}));

    const auto point = Point<Ground>::from_vec(coordinates);
    const auto vector = Vector<Ground>::from_vec(coordinates);
    EXPECT_EQ(point.to_vec(), coordinates);
    EXPECT_EQ(vector.to_vec(), coordinates);
}

TEST(SpecAngleTest, UsesExplicitUnitsAndExplicitNormalization) {
    const auto half_turn = Angle::from_degrees(180.0);
    EXPECT_NEAR(half_turn.to_radians(), std::numbers::pi, 1e-12);
    EXPECT_NEAR(Angle::from_radians(-0.5).to_degrees(), -90.0 / std::numbers::pi, 1e-12);
    EXPECT_NEAR(Angle::from_degrees(450.0).normalized_unsigned().to_degrees(), 90.0, 1e-12);
    EXPECT_NEAR(Angle::from_degrees(270.0).normalized_signed().to_degrees(), -90.0, 1e-12);
}

TEST(SpecDirectionTest, ConvertsEulerZYAndDirWithDocumentedNedSigns) {
    const AziElev angles = EulerZY<Ground>{Angle::from_degrees(30.0), Angle::from_degrees(10.0)};
    const Dir<Ground> direction = to_dir(angles);
    const AziElev restored = to_euler_zy(direction);

    EXPECT_NEAR(to_vector(direction).dx(), std::cos(std::numbers::pi / 6.0) * std::cos(std::numbers::pi / 18.0), 1e-12);
    EXPECT_LT(to_vector(direction).dz(), 0.0);
    EXPECT_NEAR(restored.azimuth().to_degrees(), 30.0, 1e-12);
    EXPECT_NEAR(restored.elevation().to_degrees(), 10.0, 1e-12);
}

TEST(SpecRotationTest, ConvertsAppliesInvertsAndComposesRotations) {
    const YawPitchRoll angles = EulerZYX<Body, Ground>{Angle::from_degrees(90.0), Angle{}, Angle{}};
    const auto body_to_ground = to_rotation(angles);
    const auto ground_vector = body_to_ground(Vector<Body>{1.0, 0.0, 0.0});

    EXPECT_NEAR(ground_vector.dx(), 0.0, 1e-12);
    EXPECT_NEAR(ground_vector.dy(), 1.0, 1e-12);
    EXPECT_NEAR(ground_vector.dz(), 0.0, 1e-12);

    const auto restored_vector = body_to_ground.inverse()(ground_vector);
    EXPECT_NEAR(restored_vector.dx(), 1.0, 1e-12);
    EXPECT_NEAR(restored_vector.dy(), 0.0, 1e-12);

    const auto camera_to_body = to_rotation(EulerZYX<Camera, Body>{Angle::from_degrees(90.0), Angle{}, Angle{}});
    const Rotation<Camera, Ground> camera_to_ground = body_to_ground * camera_to_body;
    const auto composed = body_to_ground * camera_to_body;
    // Composing two rotations collapses into a single Rotation rather than
    // falling back to the generic pair composition.
    static_assert(std::same_as<std::remove_cvref_t<decltype(composed)>, Rotation<Camera, Ground>>);
    const auto camera_x = Vector<Camera>{1.0, 0.0, 0.0};
    EXPECT_NEAR(camera_to_ground(camera_x).dx(), -1.0, 1e-12);
    EXPECT_NEAR(composed(camera_x).dx(), -1.0, 1e-12);

    const Quaternion quaternion = to_quaternion(body_to_ground);
    const auto from_quaternion = to_rotation<Body, Ground>(quaternion);
    EXPECT_NEAR(from_quaternion(Vector<Body>{1.0, 0.0, 0.0}).dy(), 1.0, 1e-12);

    const auto restored_angles = YawPitchRoll::from_transform(body_to_ground);
    EXPECT_NEAR(restored_angles.yaw().to_degrees(), 90.0, 1e-12);
}

TEST(SpecTransformTest, AppliesRigidAndAffinesByGeometryKind) {
    const auto rotation = to_rotation(EulerZYX<Body, Ground>{Angle::from_degrees(90.0), Angle{}, Angle{}});
    const Rigid<Body, Ground> rigid{rotation, Translation<Body, Ground>{Vector<Ground>{10.0, 20.0, 30.0}}};

    const auto point = rigid(Point<Body>{1.0, 0.0, 0.0});
    const auto vector = rigid(Vector<Body>{1.0, 0.0, 0.0});
    EXPECT_NEAR(point.x(), 10.0, 1e-12);
    EXPECT_NEAR(point.y(), 21.0, 1e-12);
    EXPECT_NEAR(point.z(), 30.0, 1e-12);
    EXPECT_NEAR(vector.dx(), 0.0, 1e-12);
    EXPECT_NEAR(vector.dy(), 1.0, 1e-12);

    const auto original = rigid.inverse()(point);
    EXPECT_NEAR(original.x(), 1.0, 1e-12);
    EXPECT_NEAR(original.y(), 0.0, 1e-12);

    const Affine<Image, Screen> image_to_screen{Matrix<2, 2>{2.0, 0.0, 0.0, 3.0}, Vector<Screen>{100.0, 200.0}};
    const auto screen_point = image_to_screen(Point<Image>{4.0, 5.0});
    EXPECT_DOUBLE_EQ(screen_point.x(), 108.0);
    EXPECT_DOUBLE_EQ(screen_point.y(), 215.0);
    const auto image_point = image_to_screen.inverse()(screen_point);
    EXPECT_NEAR(image_point.x(), 4.0, 1e-12);
    EXPECT_NEAR(image_point.y(), 5.0, 1e-12);
}

}  // namespace
