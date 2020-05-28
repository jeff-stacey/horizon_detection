#include "math3d.h"

#include <cmath>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

void Quaternion::normalize()
{
    float magnitude = sqrtf(w*w + x*x + y*y + z*z);
    w /= magnitude;
    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
    Quaternion result;
    result.w = w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z;
    result.x = w*rhs.x + x*rhs.w + y*rhs.z - z*rhs.y;
    result.y = w*rhs.y - x*rhs.z + y*rhs.w + z*rhs.x;
    result.z = w*rhs.z + x*rhs.y - y*rhs.x + z*rhs.w;

    return result;
}

Quaternion Quaternion::inverse() const
{
    Quaternion result;
    result.w = w;
    result.x = -x;
    result.y = -y;
    result.z = -z;

    return result;
}

void Quaternion::to_matrix(float* matrix)
{
    // assuming magnitude 1
    matrix[0] = 1 - 2 * (y*y + z*z);
    matrix[1] = 2 * (x*y - z*w);
    matrix[2] = 2 * (x*z + y*w);
    matrix[3] = 2 * (x*y + z*w);
    matrix[4] = 1 - 2 * (x*x + z*z);
    matrix[5] = 2 * (y*z - x*w);
    matrix[6] = 2 * (x*z - y*w);
    matrix[7] = 2 * (y*z + x*w);
    matrix[8] = 1 - 2 * (x*x + y*y);
}

void Quaternion::print()
{
    cout << w << ", "
         << x << ", "
         << y << ", "
         << z << endl;
}

Quaternion Quaternion::roll(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine   = sinf(0.5f * angle);

    Quaternion result;
    result.w = cosine;
    result.z = sine;

    return result;
}

Quaternion Quaternion::pitch(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine   = sinf(0.5f * angle);

    Quaternion result;
    result.w = cosine;
    result.x = sine;

    return result;
}

Quaternion Quaternion::yaw(float angle)
{
    float cosine = cosf(0.5f * angle);
    float sine   = sinf(0.5f * angle);

    Quaternion result;
    result.w = cosine;
    result.y = sine;

    return result;
}

Vec3 operator*(float lhs, const Vec3& rhs)
{
    return Vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}
