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

#include <math.h>

#define d 80
#define E_RADIUS 6378.136
#define ALTITUDE 500 // TODO: we should pass this as a parameter
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

void find_pitch_quat(const float circle_params[3], Quaternion* result)
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

	float theta = atanf((k/d)*tan(FOV/2)) + asinf(E_RADIUS/(E_RADIUS+ALTITUDE));
        result->w = cosf(0.5f*theta);
        result->x = sinf(0.5f*theta);
        result->y = 0.0f;
        result->z = 0.0f;
}

float find_yaw_quat(const float mag[3], const Quaternion* R, const Quaternion* T)
{
#if 0
	/*
	mag is the 3 dimensional magnetometer vector
	T is the affine transformation matrix
	th_x is the pitch angle (rotation about x, from camera boresight to horizon vertex)
	th_z is the roll angle (rotation about -z, between camera vertical and horizon vertex to centre screen)
	*/

	//put magnetometer into local array
	//I'm doing this because I dont know where the magnetometer value will be in memory
	//and I dont want to mess with it if we pass the reference to it directly
	float magnet[3];

	//transform magnetometer value into camera frame of reference
	multiply33by31(T, mag, magnet);

	//rotate magnetometer value around x and then around z
	float Rx[3][3] = {{1, 0, 0}, {0, cos(theta_x), -1*sin(theta_x)}, {0, sin(theta_x), cos(theta_x)}};
	float Rz[3][3] = {{cos(theta_z), -1*sin(theta_z), 0}, {sin(theta_z), cos(theta_z), 0}, {0, 0, 1}};

	float m_temp[3];

	multiply33by31(Rx, magnet, m_temp);
	multiply33by31(Rz, m_temp, magnet);

	//magnetometer value is in x-z plane
	//yaw should be angle between -z and magnet
	//[0,0,-1]dot[magx,magy,magz]
	float theta_y = acos(-1*magnet[2]);

	//determine the sign of the angle
	float A[2][2] = {{0, magnet[0]}, {-1, magnet[2]}};

	if(det2(A) < 0){
		theta_y = -1*theta_y;
	}

	return theta_y;

#else
    
    float nadir[3] = {-R[0][2], -R[1][2], -R[2][2]};
    
    float field_strength = norm(mag);

    // Project the magnetometer reading onto the plane normal
    // to the nadir vector. This is the direction we are treating
    // as north.
    float north[3] = {
        mag[0] - nadir[0]*mag[0] / field_strength,
        mag[1] - nadir[1]*mag[1] / field_strength,
        mag[2] - nadir[2]*mag[2] / field_strength
    };

    // Normalize the north vector
    float north_norm = norm(north);
    north[0] /= north_norm;
    north[1] /= north_norm;
    north[2] /= north_norm;

    // Rotate xy axes into the same plane
    float x[3] = {1.0f, 0.0f, 0.0f};
    float y[3] = {0.0f, 1.0f, 0.0f};

    quaternion_rotate(R, x);
    quaternion_rotate(R, y);

    // Dot these vectors to find the angle
    float sin_angle_to_north = -north[0]*x[0] - north[1]*x[1] - north[2]*x[2];
    float cos_angle_to_north = north[0]*y[0] + north[1]*y[1] + north[2]*y[2];

    dprintf("angle from north %f\n", acosf(cos_angle_from_north));
    
#endif
}

void find_nadir(const float results[3], float nadir[3])
{
#if 0
	//result is an array containing circle parameters (x_0,y_0,r)
	float theta_x = -1*find_pitch(results);
	float theta_z = -1*find_roll(results);

	float Rx[3][3] = {{1, 0, 0}, {0, cos(theta_x), -1*sin(theta_x)}, {0, sin(theta_x), cos(theta_x)}};
	float Rz[3][3] = {{cos(theta_z), -1*sin(theta_z), 0}, {sin(theta_z), cos(theta_z), 0}, {0, 0, 1}};

	float nad[3] = {0, 0, -1};


	//rotate nadir by theta_x about x
	multiply33by31(Rx, nad, nadir);

	//rotate nadir by theta_z about z
	multiply33by31(Rz, nadir, nad);


	nadir[0] = nad[0];
	nadir[1] = nad[1];
	nadir[2] = nad[2];
#else
        Quaternion roll_quat;
        Quaternion pitch_quat;

        find_pitch_quat(results, &pitch_quat);
        find_roll_quat(results, &roll_quat);

        pitch_quat.x *= -1.0f;
        pitch_quat.y *= -1.0f;
        pitch_quat.z *= -1.0f;

        //roll_quat.x *= -1.0f;
        //roll_quat.y *= -1.0f;
        //roll_quat.z *= -1.0f;

        Quaternion overall_transformation = quaternion_multiply(&roll_quat, &pitch_quat);

        find_yaw_quat(

        nadir[0] = 0.0f;
        nadir[1] = 0.0f;
        nadir[2] = -1.0f;
        
        quaternion_rotate(&overall_transformation, nadir);
#endif
}
