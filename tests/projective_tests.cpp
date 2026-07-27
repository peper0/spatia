#include <gtest/gtest.h>

#include <cmath>
#include <concepts>
#include <stdexcept>
#include <type_traits>

#include "spatia/spatia.hpp"

namespace {

using namespace spatia;

constexpr double tolerance = 1e-9;

struct Space {
    static constexpr std::size_t dimension = 3;
};

struct OtherSpace {
    static constexpr std::size_t dimension = 3;
};

struct Plane {
    static constexpr std::size_t dimension = 2;
};

template <class Value, class... Expected>
void expect_coordinates_near(const Value& actual, Expected... expected_values) {
    const double expected[]{static_cast<double>(expected_values)...};
    for (std::size_t index = 0; index < sizeof...(Expected); ++index) {
        EXPECT_NEAR(actual[index], expected[index], tolerance);
    }
}

Rigid<Space, OtherSpace> sample_pose() {
    const auto rotation = to_rotation(EulerZYX<Space, OtherSpace>{Angle::from_degrees(90.0), Angle{}, Angle{}});
    return {rotation, Translation<Space, OtherSpace>{Vector<OtherSpace>{1.0, 2.0, 3.0}}};
}

TEST(MatrixUtilsTest, InvertsMatricesOfAnyDimension) {
    const SquareMatrix<4> matrix{2.0, 0.0, 0.0, 1.0, 0.0, 3.0, 0.0, -2.0, 0.0, 0.0, 4.0, 5.0, 0.0, 0.0, 0.0, 1.0};
    const auto product = matrix * inverse(matrix);
    const auto identity = identity_matrix<4>();

    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            EXPECT_NEAR(product(row, column), identity(row, column), tolerance);
        }
    }

    const SquareMatrix<4> singular{1.0, 2.0, 3.0, 4.0, 2.0, 4.0, 6.0, 8.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    EXPECT_THROW(static_cast<void>(inverse(singular)), std::invalid_argument);
}

TEST(ProjectiveTest, AgreesWithTheAffineTransformItWidens) {
    const Affine<Space, OtherSpace> affine{SquareMatrix<3>{2.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 4.0},
                                           Vector<OtherSpace>{1.0, 2.0, 3.0}};
    const auto projective = to_projective(affine);
    const Point<Space> input{1.0, 1.0, 1.0};

    const auto expected = affine(input);
    expect_coordinates_near(projective(input), expected.x(), expected.y(), expected.z());
}

TEST(ProjectiveTest, RoundTripsThroughItsInverse) {
    const auto projective = to_projective(sample_pose());
    const Point<Space> input{4.0, -1.0, 0.5};

    const auto moved = projective(input);
    expect_coordinates_near(projective.inverse()(moved), input.x(), input.y(), input.z());
}

TEST(ProjectiveTest, MapsLinesToLines) {
    const auto projective = to_projective(sample_pose());
    const auto line = line_through(Point<Space>{0.0, 0.0, 1.0}, dir_of(Vector<Space>{1.0, 1.0, 0.0}));

    const auto image = projective(line, Tag<Line<OtherSpace>>{});

    // Points of the original line must land on the image line, which here is
    // exact because the pose is rigid.
    for (const Scalar distance_along : {-2.0, 0.5, 3.0}) {
        const Point<Space> on_line = closest_point_to_origin(line) + dir_of(line) * distance_along;
        const Point<OtherSpace> image_point = projective(on_line);
        const auto offset = image_point - closest_point_to_origin(image);
        const auto along = to_vector(dir_of(image)) * dot(offset, to_vector(dir_of(image)));
        EXPECT_NEAR(norm(offset - along), 0.0, tolerance);
    }
}

TEST(ProjectiveTest, PerspectiveProjectionBecomesProjectiveWhenComposed) {
    const PerspectiveProjection<OtherSpace, Plane> projection{100.0, 100.0, 320.0, 240.0};
    const auto pose = sample_pose();

    const auto composed = projection * pose;
    static_assert(std::same_as<std::remove_cvref_t<decltype(composed)>, Projective<Space, Plane>>);

    const Point<Space> input{2.0, 0.0, 1.0};
    const auto expected = projection(pose(input));
    expect_coordinates_near(composed(input), expected.x(), expected.y());

    // Composing with a rotation or a plain projective transform also stays
    // projective.
    const auto rotation = to_rotation(EulerZYX<OtherSpace, OtherSpace>{Angle{}, Angle{}, Angle{}});
    static_assert(std::same_as<std::remove_cvref_t<decltype(projection * rotation)>, Projective<OtherSpace, Plane>>);
    static_assert(
        std::same_as<std::remove_cvref_t<decltype(projection * to_projective(pose))>, Projective<Space, Plane>>);
}

TEST(NarrowingConversionTest, AcceptsTransformsThatSatisfyTheNarrowerInvariant) {
    const auto pose = sample_pose();
    const auto affine = to_affine(pose);

    const auto narrowed = to_rigid(affine);
    expect_coordinates_near(narrowed.translation(), 1.0, 2.0, 3.0);

    const Translation<Space, OtherSpace> shift{Vector<OtherSpace>{5.0, 6.0, 7.0}};
    const auto only_translation = to_translation(to_rigid(shift));
    expect_coordinates_near(only_translation.translation(), 5.0, 6.0, 7.0);

    const auto only_rotation = to_rotation(to_rigid(pose.rotation()));
    EXPECT_NEAR(only_rotation.to_matrix()(1, 0), 1.0, tolerance);
}

TEST(NarrowingConversionTest, RejectsTransformsThatViolateTheNarrowerInvariant) {
    const Affine<Space, OtherSpace> scaling{SquareMatrix<3>{2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0},
                                            Vector<OtherSpace>{}};
    EXPECT_THROW(static_cast<void>(to_rigid(scaling)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(to_rotation(scaling)), std::invalid_argument);

    const auto pose = sample_pose();
    // The pose both rotates and translates, so neither narrowing applies.
    EXPECT_THROW(static_cast<void>(to_rotation(pose)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(to_translation(pose)), std::invalid_argument);

    // A generous tolerance accepts a slightly non-orthonormal linear part.
    const Affine<Space, OtherSpace> almost_rigid{SquareMatrix<3>{1.0 + 1e-7, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0},
                                                 Vector<OtherSpace>{}};
    EXPECT_THROW(static_cast<void>(to_rigid(almost_rigid)), std::invalid_argument);
    EXPECT_NO_THROW(static_cast<void>(to_rigid(almost_rigid, 1e-5)));
}

}  // namespace
