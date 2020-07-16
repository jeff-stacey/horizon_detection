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

float find_roll(const float result[3])
{
	//result is an array containing circle parameters (x_0,y_0,r)
	//find roll (theta_z)

	//vector v1 from circle centre to image centre
	//vertical unit vector v2
	Vec2D v1;
	Vec2D v2;

	v1.x = 0 - result[0];
	v1.y = 0 - result[1];
	float v1norm = norm(&v1);
	v1.x = v1.x/v1norm;
	v1.y = v1.y/v1norm;

	v2.x = 0;
	v2.y = 1;

	//find angle between v1 and v2
	float theta_z = acos(v1.x*v2.x + v1.y*v2.y);

	//determine the sign of the angle
	float A[2][2] = {{v1.x, v2.x}, {v1.y, v2.y}};

	if(det2(A) < 0){
		theta_z = -1*theta_z;
	}

	return theta_z;
}

float find_pitch(const float results[3])
{
	//find pitch (theta_x)

	Vec2D v1;

	v1.x = 0 - results[0];
	v1.y = 0 - results[1];
	float v1norm = norm(&v1);
	//find vertex
	Vec2D vert;
	vert.x = (v1.x/v1norm)*results[2] + results[0];
	vert.y = (v1.y/v1norm)*results[2] + results[1];

	float k;

	//check if image centre is inside horizon
	if(results[2] > norm(&v1)){
		//centre is inside the horizon
		//k is negative
		k = -1*norm(&vert);
	}else{
		//k is postive
		k = norm(&vert);
	}

	return atan((k/d)*tan(FOV/2)) + asin(E_RADIUS/(E_RADIUS+ALTITUDE));
}

void find_nadir(const float results[3], float nadir[3])
{
	//result is an array containing circle parameters (x_0,y_0,r)
	float theta_x = -1*find_pitch(results);
	float theta_z = -1*find_roll(results);

	float Rx[3][3] = {{1, 0, 0}, {0, cos(theta_x), -1*sin(theta_x)}, {0, sin(theta_x), cos(theta_x)}};
	float Rz[3][3] = {{cos(theta_z), -1*sin(theta_z), 0}, {sin(theta_z), cos(theta_z), 0}, {0, 0, 1}};

	float nad[3] = {0, 0, 1};


	//rotate nadir by theta_x about x
	multiply33by31(Rx, nad, nadir);

	//rotate nadir by theta_z about z
	multiply33by31(Rz, nadir, nad);


	nadir[0] = nad[0];
	nadir[1] = nad[1];
	nadir[2] = nad[2];
}
