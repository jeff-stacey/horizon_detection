#include <stdio.h>
#include <stdint.h>
#include "math.h"

typedef struct
{
    float x;
    float y;

} Vec2D;

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
            printf("%f ", A[i][j]);
        }
        printf("\n");
    }
}

void print3(const float A[3])
{
	for(int i=0; i<3; i++){
		printf("%f\n", A[i]);
	}
}
