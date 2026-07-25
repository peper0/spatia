#include <gtest/gtest.h>

#include <cmath>
#include <concepts>
#include <numbers>
#include <stdexcept>
#include <type_traits>

#include "geometry2/math/rotation.hpp"
#include "geometry2/math/vector.hpp"
#include "geometry2/primitives/coordinate_system.hpp"
#include "geometry2/transforms/transforms.hpp"

namespace {

namespace gmath = geometry2::math;
using namespace geometry2;

constexpr double tolerance = 1e-9;

template <std::size_t Rows, std::size_t Columns, class Scalar>
void expect_matrix_near(const gmath::Matrix<Rows, Columns, Scalar>& actual,
                        const gmath::Matrix<Rows, Columns, Scalar>& expected) {
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            EXPECT_NEAR(actual(row, column), expected(row, column), tolerance);
        }
    }
}

template <class Value, class... Expected>
void expect_coordinates_near(const Value& actual, Expected... expected_values) {
    const double expected[]{static_cast<double>(expected_values)...};
    for (std::size_t index = 0; index < sizeof...(Expected); ++index) {
        EXPECT_NEAR(actual[index], expected[index], tolerance);
    }
}

struct FrameA : CoordinateSystem<FrameA, 3> {};
struct FrameB : CoordinateSystem<FrameB, 3> {};
struct FrameC : CoordinateSystem<FrameC, 3> {};

struct PlaneA : CoordinateSystem<PlaneA, 2> {};
struct PlaneB : CoordinateSystem<PlaneB, 2> {};
struct PlaneC : CoordinateSystem<PlaneC, 2> {};

template <class Angle>
concept AcceptedRotationAngle =
    requires(Angle angle) { gmath::rotation_x(angle); };

static_assert(AcceptedRotationAngle<gmath::Radians<>>);
static_assert(!AcceptedRotationAngle<double>);
static_assert(!std::convertible_to<double, gmath::Radians<>>);

TEST(RadiansTest, IsStronglyTypedAndSupportsAngleArithmetic) {
    constexpr gmath::Radians angle{0.25};
    constexpr auto sum = angle + angle;
    constexpr auto scaled = 4 * angle;

    static_assert(sum.value() == 0.5);
    static_assert(scaled.value() == 1.0);
    EXPECT_DOUBLE_EQ((-angle).value(), -0.25);
}

TEST(QuaternionTest, UsesScalarFirstHamiltonActiveRotationConvention) {
    const double half_sqrt_two = std::sqrt(0.5);
    const gmath::Quaternion quarter_turn_z{half_sqrt_two, 0.0, 0.0,
                                           half_sqrt_two};
    const gmath::Quaternion quarter_turn_x{half_sqrt_two, half_sqrt_two, 0.0,
                                           0.0};

    const auto rotated = gmath::apply_matrix<gmath::Vector3<>>(
        gmath::to_rotation_matrix(quarter_turn_z),
        gmath::Vector3<>{1.0, 0.0, 0.0});
    expect_coordinates_near(rotated, 0.0, 1.0, 0.0);

    // Hamilton product composition is right-to-left: qz * qx applies qx,
    // then qz, exactly like multiplying the corresponding column-vector
    // rotation matrices.
    expect_matrix_near(
        gmath::to_rotation_matrix(quarter_turn_z * quarter_turn_x),
        gmath::to_rotation_matrix(quarter_turn_z) *
            gmath::to_rotation_matrix(quarter_turn_x));

    EXPECT_THROW(
        static_cast<void>(gmath::to_rotation_matrix(gmath::Quaternion{})),
        std::invalid_argument);
}

TEST(RollPitchYawTest, UsesRzYawRyPitchRxRollConvention) {
    const gmath::RollPitchYaw angles{gmath::Radians{0.3}, gmath::Radians{-0.4},
                                     gmath::Radians{0.7}};
    const auto expected = gmath::rotation_z(angles.yaw) *
                          gmath::rotation_y(angles.pitch) *
                          gmath::rotation_x(angles.roll);

    expect_matrix_near(gmath::to_rotation_matrix(angles), expected);
    expect_matrix_near(gmath::to_rotation_matrix(gmath::to_quaternion(angles)),
                       expected);

    const auto recovered = gmath::to_roll_pitch_yaw(expected);
    EXPECT_NEAR(recovered.roll.value(), angles.roll.value(), tolerance);
    EXPECT_NEAR(recovered.pitch.value(), angles.pitch.value(), tolerance);
    EXPECT_NEAR(recovered.yaw.value(), angles.yaw.value(), tolerance);

    const auto recovered_from_quaternion =
        gmath::to_roll_pitch_yaw(gmath::to_quaternion(angles));
    EXPECT_NEAR(recovered_from_quaternion.roll.value(), angles.roll.value(),
                tolerance);
    EXPECT_NEAR(recovered_from_quaternion.pitch.value(), angles.pitch.value(),
                tolerance);
    EXPECT_NEAR(recovered_from_quaternion.yaw.value(), angles.yaw.value(),
                tolerance);
}

TEST(RotationConversionTest, RoundTripsQuaternionMatrixAndGimbalLockRpy) {
    const gmath::RollPitchYaw gimbal_locked{
        gmath::Radians{0.4}, gmath::Radians{std::numbers::pi_v<double> / 2.0},
        gmath::Radians{-0.3}};
    const auto rotation = gmath::to_rotation_matrix(gimbal_locked);

    const auto quaternion = gmath::to_quaternion(rotation);
    expect_matrix_near(gmath::to_rotation_matrix(quaternion), rotation);
    EXPECT_GE(quaternion.w, 0.0);

    const auto canonical_angles = gmath::to_roll_pitch_yaw(rotation);
    EXPECT_NEAR(canonical_angles.roll.value(), 0.0, tolerance);
    EXPECT_NEAR(canonical_angles.pitch.value(),
                std::numbers::pi_v<double> / 2.0, tolerance);
    expect_matrix_near(gmath::to_rotation_matrix(canonical_angles), rotation);

    // q and -q encode the same physical rotation.
    expect_matrix_near(gmath::to_rotation_matrix(quaternion),
                       gmath::to_rotation_matrix(-quaternion));
}

TEST(MatrixTest, InvertsThreeByThreeMatricesAndRejectsSingularOnes) {
    const gmath::SquareMatrix<3> matrix{1.0, 2.0, 3.0, 0.0, 1.0,
                                        4.0, 5.0, 6.0, 0.0};
    const gmath::SquareMatrix<3> expected_inverse{-24.0, 18.0, 5.0, 20.0, -15.0,
                                                  -4.0,  -5.0, 4.0, 1.0};

    expect_matrix_near(gmath::inverse(matrix), expected_inverse);
    expect_matrix_near(matrix * gmath::inverse(matrix),
                       gmath::identity_matrix<3, double>());

    EXPECT_THROW(static_cast<void>(gmath::inverse(gmath::SquareMatrix<3>{
                     1.0, 2.0, 3.0, 2.0, 4.0, 6.0, 3.0, 6.0, 9.0})),
                 std::invalid_argument);
}

TEST(TransformCompositionTest, RotationFollowedByRotationStaysRotation) {
    const RotationTransform3<FrameA, FrameB> first{
        gmath::rotation_z(gmath::Radians{0.3})};
    const RotationTransform3<FrameB, FrameC> second{
        gmath::rotation_y(gmath::Radians{-0.4})};
    const auto combined = compose(first, second);

    static_assert(std::same_as<std::remove_cvref_t<decltype(combined)>,
                               RotationTransform3<FrameA, FrameC>>);

    const FrameA::Vector input{1.0, 2.0, 3.0};
    const auto sequential =
        second(first(input, Tag<FrameB::Vector>{}), Tag<FrameC::Vector>{});
    const auto direct = combined(input, Tag<FrameC::Vector>{});
    expect_coordinates_near(direct, sequential[0], sequential[1],
                            sequential[2]);
}

TEST(TransformCompositionTest, SelectedRigidCombinationsStayRigid) {
    const RotationTransform3<FrameA, FrameB> rotation_ab{
        gmath::rotation_z(gmath::Radians{0.3})};
    const RotationTransform3<FrameB, FrameC> rotation_bc{
        gmath::rotation_y(gmath::Radians{-0.4})};
    const RigidTransform3<FrameA, FrameB> rigid_ab{
        gmath::rotation_z(gmath::Radians{0.3}), FrameB::Vector{1.0, 2.0, 3.0}};
    const RigidTransform3<FrameB, FrameC> rigid_bc{
        gmath::rotation_y(gmath::Radians{-0.4}), FrameC::Vector{4.0, 5.0, 6.0}};
    const FrameA::Point input{2.0, -1.0, 0.5};

    const auto rotation_then_rigid = compose(rotation_ab, rigid_bc);
    const auto rigid_then_rotation = compose(rigid_ab, rotation_bc);
    const auto rigid_then_rigid = compose(rigid_ab, rigid_bc);

    static_assert(
        std::same_as<std::remove_cvref_t<decltype(rotation_then_rigid)>,
                     RigidTransform3<FrameA, FrameC>>);
    static_assert(
        std::same_as<std::remove_cvref_t<decltype(rigid_then_rotation)>,
                     RigidTransform3<FrameA, FrameC>>);
    static_assert(std::same_as<std::remove_cvref_t<decltype(rigid_then_rigid)>,
                               RigidTransform3<FrameA, FrameC>>);

    const auto expected_rotation_then_rigid = rigid_bc(
        rotation_ab(input, Tag<FrameB::Point>{}), Tag<FrameC::Point>{});
    const auto expected_rigid_then_rotation = rotation_bc(
        rigid_ab(input, Tag<FrameB::Point>{}), Tag<FrameC::Point>{});
    const auto expected_rigid_then_rigid =
        rigid_bc(rigid_ab(input, Tag<FrameB::Point>{}), Tag<FrameC::Point>{});

    const auto actual_rotation_then_rigid =
        rotation_then_rigid(input, Tag<FrameC::Point>{});
    const auto actual_rigid_then_rotation =
        rigid_then_rotation(input, Tag<FrameC::Point>{});
    const auto actual_rigid_then_rigid =
        rigid_then_rigid(input, Tag<FrameC::Point>{});

    expect_coordinates_near(
        actual_rotation_then_rigid, expected_rotation_then_rigid[0],
        expected_rotation_then_rigid[1], expected_rotation_then_rigid[2]);
    expect_coordinates_near(
        actual_rigid_then_rotation, expected_rigid_then_rotation[0],
        expected_rigid_then_rotation[1], expected_rigid_then_rotation[2]);
    expect_coordinates_near(
        actual_rigid_then_rigid, expected_rigid_then_rigid[0],
        expected_rigid_then_rigid[1], expected_rigid_then_rigid[2]);
}

TEST(TransformCompositionTest, AffineFollowedByAffineStaysAffine) {
    const AffineTransform2<PlaneA, PlaneB> first{
        gmath::SquareMatrix<2>{2.0, 0.0, 0.0, 3.0}, PlaneB::Vector{1.0, 2.0}};
    const AffineTransform2<PlaneB, PlaneC> second{
        gmath::SquareMatrix<2>{1.0, 0.5, -0.25, 1.0},
        PlaneC::Vector{-3.0, 4.0}};
    const auto combined = compose(first, second);

    static_assert(std::same_as<std::remove_cvref_t<decltype(combined)>,
                               AffineTransform2<PlaneA, PlaneC>>);

    const PlaneA::Point input{2.0, -1.0};
    const auto sequential =
        second(first(input, Tag<PlaneB::Point>{}), Tag<PlaneC::Point>{});
    const auto direct = combined(input, Tag<PlaneC::Point>{});
    expect_coordinates_near(direct, sequential[0], sequential[1]);
}

}  // namespace
