#include <gtest/gtest.h>

#include "spatia/geometry/point.hpp"
#include "spatia/testing/matchers.hpp"

namespace {

using namespace spatia;
using ::testing::Not;

struct Space {
    static constexpr std::size_t dimension = 3;
};

TEST(MatrixNearTest, ChecksEveryEntryOfRectangularMatrices) {
    const Matrix<2, 3> expected{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    const Matrix<2, 3> nearby{1.01, 1.99, 3.01, 3.99, 5.01, 5.99};

    EXPECT_THAT(nearby, matrix_near(expected, 0.02));
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t column = 0; column < 3; ++column) {
            auto different = expected;
            different(row, column) += 0.1;
            EXPECT_THAT(different, Not(matrix_near(expected, 0.02)));
        }
    }
}

TEST(CoordinatesNearTest, ChecksEveryRawCoordinate) {
    const Vec<4> expected{1.0, 2.0, 3.0, 4.0};
    const Vec<4> nearby{0.99, 2.01, 2.99, 4.01};

    EXPECT_THAT(nearby, coordinates_near(expected, 0.02));
    for (std::size_t index = 0; index < 4; ++index) {
        auto different = expected;
        different[index] -= 0.1;
        EXPECT_THAT(different, Not(coordinates_near(expected, 0.02)));
    }
}

TEST(CoordinatesNearTest, MatchesPointsUsingTheirCoordinates) {
    const Point<Space> expected{1.0, 2.0, 3.0};
    const Point<Space> nearby{1.01, 1.99, 3.01};
    const Point<Space> different{1.0, 2.0, 3.1};

    EXPECT_THAT(nearby, coordinates_near(expected, 0.02));
    EXPECT_THAT(different, Not(coordinates_near(expected, 0.02)));
}

TEST(CoordinatesNearTest, MatchesVectorsUsingTheirCoordinates) {
    const Vector<Space> expected{1.0, 2.0, 3.0};
    const Vector<Space> nearby{0.99, 2.01, 2.99};
    const Vector<Space> different{1.1, 2.0, 3.0};

    EXPECT_THAT(nearby, coordinates_near(expected, 0.02));
    EXPECT_THAT(different, Not(coordinates_near(expected, 0.02)));
}

}  // namespace
