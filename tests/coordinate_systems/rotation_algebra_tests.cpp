#include <gtest/gtest.h>

#include <cmath>
#include <concepts>
#include <numbers>
#include <stdexcept>
#include <type_traits>

#include "spatia/geometry/coordinate_system.hpp"
#include "spatia/spatia.hpp"
#include "spatia/transforms.hpp"

namespace {

using namespace spatia;

constexpr double tolerance = 1e-9;

template <std::size_t Rows, std::size_t Columns>
void expect_matrix_near(const Matrix<Rows, Columns>& actual, const Matrix<Rows, Columns>& expected) {
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

TEST(QuaternionTest, UsesScalarFirstHamiltonActiveRotationConvention) {
    const double half_sqrt_two = std::sqrt(0.5);
    const Quaternion quarter_turn_z{half_sqrt_two, 0.0, 0.0, half_sqrt_two};
    const Quaternion quarter_turn_x{half_sqrt_two, half_sqrt_two, 0.0, 0.0};

    const auto z_rotation = to_rotation<FrameB, FrameC>(quarter_turn_z);
    const auto x_rotation = to_rotation<FrameA, FrameB>(quarter_turn_x);
    const auto quaternion_composition = to_rotation<FrameA, FrameC>(quarter_turn_z * quarter_turn_x);

    expect_matrix_near(quaternion_composition.to_matrix(), (z_rotation * x_rotation).to_matrix());
    expect_coordinates_near(to_rotation<FrameA, FrameB>(quarter_turn_z)(Vector<FrameA>{1.0, 0.0, 0.0}), 0.0, 1.0, 0.0);

    EXPECT_THROW(static_cast<void>(to_rotation<FrameA, FrameB>(Quaternion{})), std::invalid_argument);
}

TEST(EulerTest, RoundTripsZyxAndCanonicalizesGimbalLock) {
    const EulerZYX<FrameA, FrameB> angles{Angle::from_radians(0.7), Angle::from_radians(-0.4),
                                          Angle::from_radians(0.3)};
    const auto rotation = to_rotation(angles);
    const auto recovered = to_euler_zyx(rotation);

    EXPECT_NEAR(recovered.x().to_radians(), angles.x().to_radians(), tolerance);
    EXPECT_NEAR(recovered.y().to_radians(), angles.y().to_radians(), tolerance);
    EXPECT_NEAR(recovered.z().to_radians(), angles.z().to_radians(), tolerance);

    const auto from_quaternion = to_rotation<FrameA, FrameB>(to_quaternion(rotation));
    expect_matrix_near(from_quaternion.to_matrix(), rotation.to_matrix());

    const auto locked = to_rotation(EulerZYX<FrameA, FrameB>{
        Angle::from_radians(-0.3), Angle::from_radians(std::numbers::pi / 2.0), Angle::from_radians(0.4)});
    const auto canonical = to_euler_zyx(locked);
    EXPECT_NEAR(canonical.x().to_radians(), 0.0, tolerance);
    EXPECT_NEAR(canonical.y().to_radians(), std::numbers::pi / 2.0, tolerance);
    expect_matrix_near(to_rotation(canonical).to_matrix(), locked.to_matrix());
}

TEST(MatrixTest, InvertsThreeByThreeMatricesAndRejectsSingularOnes) {
    const SquareMatrix<3> matrix{1.0, 2.0, 3.0, 0.0, 1.0, 4.0, 5.0, 6.0, 0.0};
    const SquareMatrix<3> expected_inverse{-24.0, 18.0, 5.0, 20.0, -15.0, -4.0, -5.0, 4.0, 1.0};

    expect_matrix_near(inverse(matrix), expected_inverse);
    expect_matrix_near(matrix * inverse(matrix), identity_matrix<3>());

    EXPECT_THROW(static_cast<void>(inverse(SquareMatrix<3>{1.0, 2.0, 3.0, 2.0, 4.0, 6.0, 3.0, 6.0, 9.0})),
                 std::invalid_argument);
}

TEST(TransformCompositionTest, RotationFollowedByRotationStaysRotation) {
    const Rotation<FrameA, FrameB> first =
        to_rotation(EulerZYX<FrameA, FrameB>{Angle::from_radians(0.3), Angle{}, Angle{}});
    const Rotation<FrameB, FrameC> second =
        to_rotation(EulerZYX<FrameB, FrameC>{Angle{}, Angle::from_radians(-0.4), Angle{}});
    const auto combined = second * first;

    static_assert(std::same_as<std::remove_cvref_t<decltype(combined)>, Rotation<FrameA, FrameC>>);

    const FrameA::Vector input{1.0, 2.0, 3.0};
    const auto sequential = second(first(input));
    expect_coordinates_near(combined(input), sequential[0], sequential[1], sequential[2]);
}

TEST(TransformCompositionTest, SelectedRigidCombinationsStayRigid) {
    const auto rotation_ab = to_rotation(EulerZYX<FrameA, FrameB>{Angle::from_radians(0.3), Angle{}, Angle{}});
    const auto rotation_bc = to_rotation(EulerZYX<FrameB, FrameC>{Angle{}, Angle::from_radians(-0.4), Angle{}});
    const Rigid<FrameA, FrameB> rigid_ab{rotation_ab, Translation<FrameA, FrameB>{FrameB::Vector{1.0, 2.0, 3.0}}};
    const Rigid<FrameB, FrameC> rigid_bc{rotation_bc, Translation<FrameB, FrameC>{FrameC::Vector{4.0, 5.0, 6.0}}};
    const FrameA::Point input{2.0, -1.0, 0.5};

    const auto rotation_then_rigid = rigid_bc * rotation_ab;
    const auto rigid_then_rotation = rotation_bc * rigid_ab;
    const auto rigid_then_rigid = rigid_bc * rigid_ab;

    static_assert(std::same_as<std::remove_cvref_t<decltype(rotation_then_rigid)>, Rigid<FrameA, FrameC>>);
    static_assert(std::same_as<std::remove_cvref_t<decltype(rigid_then_rotation)>, Rigid<FrameA, FrameC>>);
    static_assert(std::same_as<std::remove_cvref_t<decltype(rigid_then_rigid)>, Rigid<FrameA, FrameC>>);

    const auto expected_rotation_then_rigid = rigid_bc(rotation_ab(input, Tag<FrameB::Point>{}));
    const auto expected_rigid_then_rotation = rotation_bc.rotate_about_shared_origin(rigid_ab(input));
    const auto expected_rigid_then_rigid = rigid_bc(rigid_ab(input));

    expect_coordinates_near(rotation_then_rigid(input), expected_rotation_then_rigid[0],
                            expected_rotation_then_rigid[1], expected_rotation_then_rigid[2]);
    expect_coordinates_near(rigid_then_rotation(input), expected_rigid_then_rotation[0],
                            expected_rigid_then_rotation[1], expected_rigid_then_rotation[2]);
    expect_coordinates_near(rigid_then_rigid(input), expected_rigid_then_rigid[0], expected_rigid_then_rigid[1],
                            expected_rigid_then_rigid[2]);
}

TEST(TransformCompositionTest, AffineFollowedByAffineStaysAffine) {
    const Affine<PlaneA, PlaneB> first{SquareMatrix<2>{2.0, 0.0, 0.0, 3.0}, PlaneB::Vector{1.0, 2.0}};
    const Affine<PlaneB, PlaneC> second{SquareMatrix<2>{1.0, 0.5, -0.25, 1.0}, PlaneC::Vector{-3.0, 4.0}};
    const auto combined = second * first;
    const PlaneA::Point input{2.0, -1.0};

    static_assert(std::same_as<std::remove_cvref_t<decltype(combined)>, Affine<PlaneA, PlaneC>>);
    const auto sequential = second(first(input));
    expect_coordinates_near(combined(input), sequential[0], sequential[1]);
}

// The generic composition operator must not swallow ordinary multiplication,
// so only single-conversion transforms may satisfy its constraint.
static_assert(!SingleConversionTransform<Matrix<3, 3>>);
static_assert(!SingleConversionTransform<Vec<3>>);
static_assert(!SingleConversionTransform<Quaternion>);
static_assert(!SingleConversionTransform<Vector<FrameA>>);
static_assert(!SingleConversionTransform<Angle>);
static_assert(!SingleConversionTransform<Rotation<FrameA, FrameB>>);
static_assert(SingleConversionTransform<decltype([](const Point<FrameA>& p) { return Point<FrameB>{p.to_vec()}; })>);

// Each conversion is one overload with a defaulted destination tag. Where a
// source type has more than one possible destination, the extra ones keep
// requiring the tag, so the plain call stays unambiguous and the deliberate
// restrictions survive.
static_assert(std::is_invocable_v<Rotation<FrameA, FrameB>, Vector<FrameA>>);
static_assert(std::is_invocable_v<Rotation<FrameA, FrameB>, Dir<FrameA>>);
static_assert(std::is_invocable_v<Rotation<FrameA, FrameB>, Line<FrameA>>);
static_assert(!std::is_invocable_v<Rotation<FrameA, FrameB>, Point<FrameA>>);
static_assert(std::is_invocable_v<Rotation<FrameA, FrameB>, Point<FrameA>, Tag<Point<FrameB>>>);
static_assert(std::same_as<std::invoke_result_t<Rigid<FrameA, FrameB>, Dir<FrameA>>, Dir<FrameB>>);
static_assert(std::same_as<std::invoke_result_t<Rigid<FrameA, FrameB>, Dir<FrameA>, Tag<Line<FrameB>>>, Line<FrameB>>);

static_assert(!std::is_invocable_v<Rotation<FrameA, FrameB>, Vector<FrameB>>);
static_assert(std::is_invocable_v<BiRotation<FrameA, FrameB>, Vector<FrameB>>);
static_assert(!std::is_invocable_v<Rigid<FrameA, FrameB>, Point<FrameB>>);
static_assert(std::is_invocable_v<BiRigid<FrameA, FrameB>, Point<FrameB>>);

TEST(BiTransformTest, RotationExposesBothDirectionsFromOneObject) {
    const auto forward =
        to_rotation(EulerZYX<FrameA, FrameB>{Angle::from_radians(0.3), Angle::from_radians(-0.2), Angle{}});
    const BiRotation<FrameA, FrameB> both{forward};

    const Vector<FrameA> input{1.0, 2.0, 3.0};
    const Vector<FrameB> rotated = both(input);
    expect_coordinates_near(rotated, forward(input)[0], forward(input)[1], forward(input)[2]);
    expect_coordinates_near(both(rotated), input[0], input[1], input[2]);

    expect_matrix_near(both.inverse().to_matrix(), forward.inverse().to_matrix());
    expect_matrix_near(to_rotation<FrameA, FrameB>(forward.to_quaternion()).to_matrix(), forward.to_matrix());
}

TEST(BiTransformTest, RigidRoundTripsPoints) {
    const auto rotation = to_rotation(EulerZYX<FrameA, FrameB>{Angle::from_radians(0.4), Angle{}, Angle{}});
    const BiRigid<FrameA, FrameB> both{rotation, Translation<FrameA, FrameB>{FrameB::Vector{1.0, 2.0, 3.0}}};

    const Point<FrameA> input{2.0, -1.0, 0.5};
    const Point<FrameB> moved = both(input);
    expect_coordinates_near(both(moved), input[0], input[1], input[2]);
}

TEST(Rotation2DTest, RotatesInThePlaneAndRoundTripsAngle) {
    const auto rotation = to_rotation<PlaneA, PlaneB>(Angle::from_degrees(90.0));
    const auto rotated = rotation(Vector<PlaneA>{1.0, 0.0});
    expect_coordinates_near(rotated, 0.0, 1.0);
    EXPECT_NEAR(to_angle(rotation).to_degrees(), 90.0, tolerance);

    const BiRotation<PlaneA, PlaneB> both{rotation};
    expect_coordinates_near(both(rotated), 1.0, 0.0);

    const Rigid<PlaneA, PlaneB> rigid{rotation, Translation<PlaneA, PlaneB>{Vector<PlaneB>{10.0, 20.0}}};
    expect_coordinates_near(rigid(Point<PlaneA>{1.0, 0.0}), 10.0, 21.0);
    expect_coordinates_near(rigid.inverse()(Point<PlaneB>{10.0, 21.0}), 1.0, 0.0);
}

TEST(TranslationTest, RetagsVectorsAndMovesOnlyPoints) {
    const Translation<PlaneA, PlaneB> first{Vector<PlaneB>{10.0, 20.0}};
    const Translation<PlaneB, PlaneC> second{Vector<PlaneC>{-2.0, 3.0}};
    const auto combined = second * first;

    expect_coordinates_near(combined(Point<PlaneA>{1.0, 2.0}), 9.0, 25.0);
    expect_coordinates_near(combined(Vector<PlaneA>{1.0, 2.0}), 1.0, 2.0);
    expect_coordinates_near(combined.inverse()(Point<PlaneC>{9.0, 25.0}), 1.0, 2.0);
}

TEST(TranslationTest, MixesWithRotationsAndRigidTransformsIntoRigid) {
    const auto quarter_turn_ab = to_rotation<PlaneA, PlaneB>(Angle::from_degrees(90.0));
    const auto quarter_turn_bc = to_rotation<PlaneB, PlaneC>(Angle::from_degrees(90.0));
    const Translation<PlaneA, PlaneB> translation_ab{Vector<PlaneB>{1.0, 2.0}};
    const Translation<PlaneB, PlaneC> translation_bc{Vector<PlaneC>{10.0, 20.0}};
    const Rigid<PlaneB, PlaneC> rigid_bc{quarter_turn_bc, translation_bc};
    const Point<PlaneA> input{3.0, 4.0};

    const auto rotation_then_translation = translation_bc * quarter_turn_ab;
    const auto translation_then_rotation = quarter_turn_bc * translation_ab;
    const auto translation_then_rigid = rigid_bc * translation_ab;

    static_assert(std::same_as<std::remove_cvref_t<decltype(rotation_then_translation)>, Rigid<PlaneA, PlaneC>>);
    static_assert(std::same_as<std::remove_cvref_t<decltype(translation_then_rotation)>, Rigid<PlaneA, PlaneC>>);
    static_assert(std::same_as<std::remove_cvref_t<decltype(translation_then_rigid)>, Rigid<PlaneA, PlaneC>>);

    // A quarter turn maps (3, 4) to (-4, 3); adding (10, 20) gives (6, 23).
    expect_coordinates_near(rotation_then_translation(input), 6.0, 23.0);
    // Translating first gives (4, 6), whose quarter turn is (-6, 4).
    expect_coordinates_near(translation_then_rotation(input), -6.0, 4.0);
    // The same rotation plus the rigid offset (10, 20) gives (4, 24).
    expect_coordinates_near(translation_then_rigid(input), 4.0, 24.0);
}

}  // namespace
