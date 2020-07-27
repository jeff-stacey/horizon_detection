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

#include "edge.h"

#include "common.h"
#include <stdint.h>
#include "math.h"

/*******************
 *    KERNELS      *
********************/

float kernel_gauss[K_DIM][K_DIM] = {
        // Gaussian Blur kernel
            {1.0/16, 2.0/16, 1.0/16},
            {2.0/16, 4.0/16, 2.0/16},
            {1.0/16, 2.0/16, 1.0/16}
        };

int16_t kernel_x[K_DIM][K_DIM] = {
        // SOBEL kernel
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}
        };

int16_t kernel_y[K_DIM][K_DIM] = {
        // SOBEL kernel
            {-1, -2, -1},
            {0, 0, 0},
            {1, 2, 1}
        };

/*******************
 *    FUNCTIONS    *
********************/

/*******************************************************
 *    2D Convolution (kernel is type: int16_t)
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K - 3x3 Convolution Kernel
 *
*******************************************************/
void conv2d(pixel A[R_DIM][C_DIM], pixel C[R_DIM][C_DIM], int16_t K[K_DIM][K_DIM]) {
    uint16_t rows = R_DIM;
    uint16_t cols = C_DIM;
    int16_t a, b, i, j;
    volatile int16_t sum = 0;

    /*Iterate through image*/
    for (i=2 ; i < rows-2 ; i++) {
        for (j=2 ; j < cols-2 ; j++){
            sum = 0;
            /* Iterate through kernel */
            for (a=-1; a < 2; a++) {
                for (b=-1; b < 2; b++) {
                    /*Add to the sum*/
                    sum += A[i+a][j+b]*K[a+1][b+1];
                }
            }
            C[i][j] = (pixel)abs(sum);
        }
    }
};

/*******************************************************
 *    2D Convolution (kernel is type: float)
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K - 3x3 Convolution Kernel
 *
*******************************************************/
void conv2dGauss(pixel A[R_DIM][C_DIM], pixel C[R_DIM][C_DIM], float K[K_DIM][K_DIM]) {
    int16_t rows = R_DIM;
    int16_t cols = C_DIM;
    int16_t a, b, i, j;
    volatile float sum = 0.0;

    /* Iterate through image */
    for (i=1 ; i < rows-1 ; i++) {
        for (j=1 ; j < cols-1 ; j++){
            sum = 0.0;
            /* Iterate through kernel */
            for (a=-1; a < 2; a++) {
                for (b=-1; b < 2; b++) {
                    /*Add to the sum*/
                    sum = sum + ((float)A[i+a][j+b])*K[a+1][b+1];
                }
            }
            /* Sum Thresholding */
            if (sum < 0) sum = 0;
            if (sum > 0x3fff) sum = 0x3fff;
            C[i][j] = (pixel)sum;
        }
    }
};

/********************************************************************
 *    Non-Maximum Suppression
 *    Inputs:  A - 120x160 Image Result Matrix
 *             G - 120x160 Gradient Magnitude Matrix
 *             T - 120x160 Gradient Phase Matrix
 *
**********************************************************************/
void nonMaxSuppression(pixel A[R_DIM][C_DIM], pixel G[R_DIM][C_DIM], float T[R_DIM][C_DIM]) {

    uint16_t i, j, q, r;
    float angle = 0;
    pixel current;

    // Iterate through matrix
    for(i = 2; i < R_DIM-2 ; i++) {
        for(j = 2; j < C_DIM-2 ; j++) {
            q = 0x3fff;
            r = 0x3fff;

            // Convert angle to degrees and scale so non-negative
            angle = T[i][j]*180/M_PI;
            if (angle < 0) angle += 180;

            // Obtain neighbor pixels in gradient direction
            if ((0 <= angle && angle < 22.5) || (157.5 <= angle && angle <= 180)) {
                // Angle 0
                q = G[i][j+1];
                r = G[i][j-1];
            } else if (22.5 <= angle && angle < 67.5) {
                // Angle 45
                q = G[i+1][j-1];
                r = G[i-1][j+1];
            } else if (67.5 <= angle && angle < 112.5) {
                // Angle 90
                q = G[i+1][j];
                r = G[i-1][j];
            } else if (112.5 <= angle && angle < 157.5) {
                // Angle 135
                q = G[i-1][j-1];
                r = G[i+1][j+1];
            }

            // Check if current pixel is a maximum in this direction
            current = G[i][j];
            if ((current >= q) && (current >= r)) {
                A[i][j] = current;
            } else {
                A[i][j] = 0;
            }

        }
    }
};

/********************************************************************
 *    Double Thresholding
 *    Inputs:        A      - 120X160 Image Matrix
 *             lowRatio  - low_thresh/high_thresh
 *             highRatio - high_thresh/maximum_value
 *
**********************************************************************/
void doubleThreshold (pixel A[R_DIM][C_DIM], float lowRatio, float highRatio) {

    // Obtain max image value to determine thresholds
    uint16_t i, j;
    pixel max = 0;
    pixel a_pix;
    for (i = 2; i < R_DIM-2 ; i++) {
        for (j = 2; j < C_DIM-2 ; j++) {
            a_pix = A[i][j];
            if (a_pix > max) max = a_pix;
        };
    };

    // Calculate Thresholds
    pixel highThresh = max*highRatio;
    pixel lowThresh  = highThresh*lowRatio;

    // Define values of weak and strong edges
    pixel weak = 0x666;
    pixel strong = 0x3fff;

    // Filter the matrix to contain only 3 possible values (strong, weak, 0)
    for (i=2; i < R_DIM-2 ; i++) {
        for (j=2; j < C_DIM-2 ; j++) {
            a_pix = A[i][j];
            if (a_pix >= highThresh) {
                A[i][j] = strong;
            } else if ((a_pix <= highThresh) && (a_pix >= lowThresh)) {
                A[i][j] = weak;
            } else {
                A[i][j] = 0;
            }
        };
    };


};

/********************************************************************
 *    Edge Tracking by Hysteresis
 *    Inputs:       A       - 120x160 Image Array
 *             strong - Value of "strong" edge pixel = 255
 *              weak  - Value of "weak" edge pixel = 25
 *
**********************************************************************/
void edgeTracking(pixel A[R_DIM][C_DIM], pixel strong, pixel weak) {

    uint16_t i, j;

    // Check around weak edges to see if they're part of a strong edge
    for (i=2; i < R_DIM-2 ; i++) {
        for (j=2; j < C_DIM-2 ; j++) {
            if (A[i][j] == weak) {
                // If any strong pixels around it, assign strong, else assign 0
                if (A[i-1][j-1] == strong || A[i-1][j] == strong || A[i-1][j+1] == strong ||
                     A[i][j-1]  == strong || A[i][j+1] == strong ||
                    A[i+1][j-1] == strong || A[i+1][j] == strong || A[i+1][j+1] == strong)
                {
                    A[i][j] = strong;
                } else {
                    A[i][j] = 0;
                }
            }

        };
    };
};

/********************************************************************
 *    Hypotenuse Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Result Matrix
 *
**********************************************************************/
void imgHypot (pixel X[R_DIM][C_DIM], pixel Y[R_DIM][C_DIM], pixel C[R_DIM][C_DIM]) {

    uint16_t i, j;
    float result;

    for (i=2; i < R_DIM-2 ; i++) {
        for (j=2; j < C_DIM-2 ; j++) {
            result =  hypotf((float)X[i][j], (float)Y[i][j]);
            C[i][j] = (pixel)result;
            
        };
    };
};

/********************************************************************
 *    Angle Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Result Matrix
 *
**********************************************************************/
void imgTheta (pixel X[R_DIM][C_DIM], pixel Y[R_DIM][C_DIM], float C[R_DIM][C_DIM]) {

    uint16_t i, j;
    float result;

    for (i=2; i < R_DIM-2 ; i++) {
        for (j=2; j < C_DIM-2 ; j++) {
            result =  atan2f((float)Y[i][j], (float)X[i][j]);
            C[i][j] = result;       
        };
    };
};

// Debugging function for comparing rows of pixels
void printRowSum(pixel A[R_DIM][C_DIM]) {

    int row_acc = 0; 

    for (int i = 0; i < R_DIM; i++)
    {
        row_acc = 0;
        for (int j = 0; j < C_DIM; j++)
        {
            row_acc += A[i][j];
        }

        dprintf("\trow sum for %3d is %d\n", i, row_acc);
    }
}

void printRowSumTheta(float A[R_DIM][C_DIM]) {

    float row_acc = 0; 

    for (int i = 0; i < R_DIM; i++)
    {
        row_acc = 0;
        for (int j = 0; j < C_DIM; j++)
        {
            row_acc += A[i][j];
        }

        dprintf("\trow sum for %3d is %f\n", i, row_acc);
    }
}

/********************************************************************
 *    Converts Edge Map into Array of Vec2D structs
 *    Inputs:     E     - Edge map image
 *             edge_ind - array of x,y structs containing
 *                        edge point indicies
 *
**********************************************************************/
uint16_t edge2Arr(pixel E[R_DIM][C_DIM], Vec2D edge_ind[NUM_PIX]) {
    uint16_t i, j;
    float k, l;
    uint16_t edge_iter = 0;
    uint16_t num_points = 0; 
    for (i=2; i < R_DIM-2 ; i++) {
        for (j=2; j < C_DIM-2 ; j++) {
            if (E[i][j] != 0) {
                // Convert so (0,0) is center of image
                k = -i + 59.5; // Row conversion (y-value)
                l = j - 79.5; // column conversion (x-value)
                // Store
                edge_ind[edge_iter].x = l;
                edge_ind[edge_iter].y = k;
                edge_iter++;
                num_points++;
            }
        }
    }
    return num_points;
};

/********************************************************************
 *    Prints out contents of edge points array
 *    Inputs:  edge_points  - array of Vec2D structures
 *
**********************************************************************/
void edgePrint(Vec2D edge_ind[], uint16_t size) {
    for(uint16_t i = 0; i < size; i++) {
        dprintf("\tedge point @ %3f,%3f\n", edge_ind[i].x, edge_ind[i].y);
    }
}
