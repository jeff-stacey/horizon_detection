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

#include "common.h"
#include "linalg.h"

#include <math.h>

float norm(Vec2D* V)
{
    return sqrt(pow(V->x,2) + pow(V->y,2));
}

float det2(float A[2][2])
{
	return A[0][0]*A[1][1] - A[1][0]*A[0][1];
}

void inv3(float A[3][3])
{
    float det = A[0][0]*A[1][1]*A[2][2] +
    			A[0][1]*A[1][2]*A[2][0] +
				A[0][2]*A[1][0]*A[2][1] -
				A[0][2]*A[1][1]*A[2][0] -
				A[0][1]*A[1][0]*A[2][2] -
				A[0][0]*A[1][2]*A[2][1];


    float invA[3][3];

    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            invA[i][j] = (A[(i+1)%3][(j+1)%3]*A[(i+2)%3][(j+2)%3] - A[(i+1)%3][(j+2)%3]*A[(i+2)%3][(j+1)%3])/det;
        }
    }

    for(int i=0; i<3; i++){
    	for(int j=0; j<3; j++){
    		A[i][j] = invA[i][j];
    	}
    }
}

void multiply33by31(const float A[3][3], const float v[3], float u[3])
{
    //multiplay 3x3 matrix A with 3x1 vector v and store in vector u

	//zero u
	for(int i=0; i<3; i++){
		u[i] = 0;
	}

    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            u[i] += A[i][j]*v[j];
        }
    }

}

void print33(const float A[3][3])
{
    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            dprintf("%f ", A[i][j]);
        }
        dprintf("\n");
    }
}

void print3(const float A[3])
{
	for(int i=0; i<3; i++){
		dprintf("%f\n", A[i]);
	}
}