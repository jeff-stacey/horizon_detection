/*
Copyright (c) 2020 Ryan Blais, Hugo Burd, Byron Kontou, and Jeff Stacey

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "attitude.h"
#include "linalg.h"
#include "common.h"

#include <math.h>

#define d 80
#define E_RADIUS 6378.136
#define FOV 0.994838

void find_roll_quat(const float circle_params[3], Quaternion* result)
{
    //result is an array containing circle parameters (x_0,y_0,r)
    //find roll (theta_z)

    //vector v1 from circle centre to image centre
    Vec2D v1;

    v1.x = -circle_params[0];
    v1.y = -circle_params[1];
    float v1norm = norm(&v1);
    v1.x = v1.x/v1norm;
    v1.y = v1.y/v1norm;

    // TODO: We can use a half angle formula here to avoid acosf
    float theta = acosf(v1.y);

    if (v1.x > 0.0f)
    {
        theta *= -1;
    }

    result->w = cosf(0.5f * theta);
    result->x = 0.0f;
    result->y = 0.0f;
    result->z = sinf(0.5f * theta);
}

void find_pitch_quat(const float circle_params[3], float altitude, Quaternion* result)
{
    //find pitch (theta_x)

    Vec2D v1;

    v1.x = -circle_params[0];
    v1.y = -circle_params[1];
    float v1norm = norm(&v1);
    //find vertex
    Vec2D vert;
    vert.x = (v1.x/v1norm)*circle_params[2] + circle_params[0];
    vert.y = (v1.y/v1norm)*circle_params[2] + circle_params[1];

    // k is the distance to the vertex
    float k;

    //check if image centre is inside horizon
    if(circle_params[2] > norm(&v1)){
        //centre is inside the horizon
        //k is negative
        k = -1*norm(&vert);
    }else{
        //k is postive
        k = norm(&vert);
    }

    float theta = atanf((k/d)*tan(FOV/2)) + asinf(E_RADIUS/(E_RADIUS+altitude));
    result->w = cosf(0.5f*theta);
    result->x = sinf(0.5f*theta);
    result->y = 0.0f;
    result->z = 0.0f;
}

void find_yaw_quat(const float mag[3], const Quaternion* R, Quaternion* result)
{
    /*
    mag is the 3 dimensional magnetometer vector
    */

    // TODO: worry about T

    float nadir[3] = {0.0f, 0.0f, -1.0f};
    quaternion_rotate(R, nadir);

    // Project the magnetometer reading onto the plane normal
    // to the nadir vector. This is the direction we are treating
    // as north in the camera reference frame.
    float mag_nadir_component = nadir[0]*mag[0] + nadir[1]*mag[1] + nadir[2]*mag[2];
    float north[3] = {
        mag[0] - mag_nadir_component*nadir[0],
        mag[1] - mag_nadir_component*nadir[1],
        mag[2] - mag_nadir_component*nadir[2]
    };
     
    // TODO: check for zero or very small north vector

    // Normalize the north vector
    float north_norm = norm3(north);
    north[0] /= north_norm;
    north[1] /= north_norm;
    north[2] /= north_norm;

    // Rotate north into the xy plane, which gives us the cos and sin of the angle
    Quaternion Rinv = quaternion_inverse(R);
    quaternion_rotate(&Rinv, north);

    float cos_theta = north[1];
    float sin_theta = -north[0];

    // TODO: We can use half angle formula to avoid acosf
    float theta = acosf(cos_theta);
    if (sin_theta < 0.0f)
    {
        theta = -theta;
    }

    dprintf("north z %f\n", north[2]);
    dprintf("north angle %f\n", theta);

    result->w = cosf(0.5f * theta);
    result->x = 0.0f;
    result->y = 0.0f;
    result->z = sinf(0.5f * theta);
}

void find_nadir(const float results[3], float altitude, float nadir[3], float mag[3], Quaternion* overall_transformation)
{
    Quaternion roll_quat;
    Quaternion pitch_quat;

    find_pitch_quat(results, altitude, &pitch_quat);
    find_roll_quat(results, &roll_quat);

    pitch_quat.x *= -1.0f;
    pitch_quat.y *= -1.0f;
    pitch_quat.z *= -1.0f;

    *overall_transformation = quaternion_multiply(&roll_quat, &pitch_quat);

    nadir[0] = 0.0f;
    nadir[1] = 0.0f;
    nadir[2] = -1.0f;

    quaternion_rotate(overall_transformation, nadir);

    Quaternion yaw_quat;
    find_yaw_quat(mag, overall_transformation, &yaw_quat);
    //yaw_quat = quaternion_inverse(&yaw_quat);
    *overall_transformation = quaternion_multiply(overall_transformation, &yaw_quat);
}
