#include <iostream>
#include <spatia/spatia.hpp>
#include <utility>

using namespace spatia;

struct GndNed {
    static constexpr std::size_t dimension = 3;
};

struct BodyNed {
    static constexpr std::size_t dimension = 3;
};

struct GndNedAtBodyOrigin {
    static constexpr std::size_t dimension = 3;
};

struct Camera {
    static constexpr std::size_t dimension = 3;
};

struct Image {
    static constexpr std::size_t dimension = 2;
};

class AziElev final : public EulerZY<GndNed> {
    using Base = EulerZY<GndNed>;

   public:
    AziElev(Base value) : Base{std::move(value)} {}

    Angle& azimuth() noexcept { return z(); }
    const Angle& azimuth() const noexcept { return z(); }
    Angle& elevation() noexcept { return y(); }
    const Angle& elevation() const noexcept { return y(); }
};

class PanTilt final : public EulerZY<BodyNed> {
    using Base = EulerZY<BodyNed>;

   public:
    PanTilt(Base value) : Base{std::move(value)} {}

    Angle& pan() noexcept { return z(); }
    const Angle& pan() const noexcept { return z(); }
    Angle& tilt() noexcept { return y(); }
    const Angle& tilt() const noexcept { return y(); }
};

class YawPitchRoll final : public EulerZYX<BodyNed, GndNed> {
    using Base = EulerZYX<BodyNed, GndNed>;

   public:
    YawPitchRoll(Base value) : Base{std::move(value)} {}

    Angle& yaw() noexcept { return z(); }
    const Angle& yaw() const noexcept { return z(); }
    Angle& pitch() noexcept { return y(); }
    const Angle& pitch() const noexcept { return y(); }
    Angle& roll() noexcept { return x(); }
    const Angle& roll() const noexcept { return x(); }

    static YawPitchRoll from_transform(const Rotation<BodyNed, GndNed>& transform) { return to_euler_zyx(transform); }
};

class PanTiltRoll final : public EulerZYX<Camera, BodyNed> {
    using Base = EulerZYX<Camera, BodyNed>;

   public:
    PanTiltRoll(Base value) : Base{std::move(value)} {}

    Angle& pan() noexcept { return z(); }
    const Angle& pan() const noexcept { return z(); }
    Angle& tilt() noexcept { return y(); }
    const Angle& tilt() const noexcept { return y(); }
    Angle& roll() noexcept { return x(); }
    const Angle& roll() const noexcept { return x(); }

    static PanTiltRoll from_transform(const Rotation<Camera, BodyNed>& transform) { return to_euler_zyx(transform); }
};

int main() {
    const Point<Image> pixel{640.0, 360.0};
    const Vector<Image> image_offset{4.0, -2.0};
    const Point<Image> shifted_pixel = pixel + image_offset;

    const AziElev target_angles = EulerZY<GndNed>{degrees(30.0), degrees(10.0)};
    const Dir<GndNed> target_direction = to_dir(target_angles);
    const AziElev restored_target = to_euler_zy(target_direction);

    const PanTilt body_angles = EulerZY<BodyNed>{degrees(15.0), degrees(-5.0)};
    const Dir<BodyNed> body_direction = to_dir(body_angles);

    const YawPitchRoll attitude = EulerZYX<BodyNed, GndNed>{degrees(20.0), degrees(3.0), degrees(-1.0)};
    const Rotation<BodyNed, GndNed> body_to_ground = to_rotation(attitude);
    const Dir<GndNed> transformed_direction = body_to_ground(body_direction);
    const YawPitchRoll restored_attitude = YawPitchRoll::from_transform(body_to_ground);
    const Quaternion attitude_quaternion = to_quaternion(body_to_ground);

    const PanTiltRoll camera_mount = EulerZYX<Camera, BodyNed>{degrees(2.0), degrees(-10.0), Angle{}};
    const Rotation<Camera, BodyNed> camera_to_body = to_rotation(camera_mount);
    const Rotation<Camera, GndNed> camera_to_ground = body_to_ground * camera_to_body;
    const Rotation<Camera, GndNed> same_composition = body_to_ground * camera_to_body;
    const Rotation<GndNed, Camera> ground_to_camera = camera_to_ground.inverse();

    const Rotation<BodyNed, GndNedAtBodyOrigin> align_body{to_matrix(attitude)};
    const Translation<GndNedAtBodyOrigin, GndNed> place_body{Point<GndNed>{100.0, 50.0, -20.0}};
    const Rigid<BodyNed, GndNed> pose = place_body * align_body;
    const Point<GndNed> sensor_in_ground = pose(Point<BodyNed>{1.0, 0.0, 0.0});

    std::cout << "pixel: " << shifted_pixel.x() << ", " << shifted_pixel.y()
              << "\nrestored azimuth: " << restored_target.azimuth().to_degrees()
              << " deg\nrestored yaw: " << restored_attitude.yaw().to_degrees()
              << " deg\nquaternion w: " << attitude_quaternion.w
              << "\nground direction x: " << to_vector(transformed_direction).dx()
              << "\nsensor north: " << sensor_in_ground.x() << "\n";

    static_cast<void>(same_composition);
    static_cast<void>(ground_to_camera);

    // These examples intentionally do not compile:
    // Vector<GndNed>{} + Vector<BodyNed>{};
    // Dir<BodyNed> wrong = to_dir(target_angles);
    // YawPitchRoll wrong = PanTiltRoll::from_transform(camera_to_body);
}
