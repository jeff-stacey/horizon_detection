#pragma once

#pragma pack(push, 1)
struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float x_, float y_, float z_): x(x_), y(y_), z(z_) {};

    float magnitude();
};
#pragma pack(pop)

Vec3 operator*(float lhs, const Vec3& rhs);
Vec3 operator+(const Vec3& lhs, const Vec3& rhs);

static_assert(sizeof(Vec3) == 3 * sizeof(float), "Vec3 has to be packed");

struct Quaternion
{
    float w = 1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    void normalize();

    Quaternion operator*(const Quaternion& rhs) const;

    Quaternion inverse() const;

    void to_matrix(float* matrix);

    void print();

    Vec3 apply_rotation(const Vec3& x);

    static Quaternion roll(float angle);
    static Quaternion pitch(float angle);
    static Quaternion yaw(float angle);
};

