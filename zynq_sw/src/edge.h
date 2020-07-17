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

#ifndef CONV_HEADER
#define CONV_HEADER

#include <stdint.h>
#include "linalg.h"

/*TODO: HELPER FUNCTIONS FOR EDGE DETECTION
    - DONE 2d convolve
    - DONE element-wise array "hypotenuse" operation
    - DONE element-wise array "theta" operation
    - DONE Non-Max suppression
    - DONE Double Thresholding
    - DONE Edge Tracking by Hysteresis
*/

/*******************
 *    DEFINES      *
********************/

/* Types */
typedef int16_t pixel;  //16-bit images

/*Image Dimensions*/
#define R_DIM 120
#define C_DIM 160
#define NUM_PIX R_DIM*C_DIM

/*Kernel Dimensions*/
#define K_DIM 3 //Unless otherwise specified, kernel is 3x3


/*******************
 *    KERNELS      *
********************/

extern float kernel_gauss[K_DIM][K_DIM];

extern int16_t kernel_x[K_DIM][K_DIM];

extern int16_t kernel_y[K_DIM][K_DIM];

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
void conv2d(pixel A[R_DIM][C_DIM], pixel C[R_DIM][C_DIM], int16_t K[K_DIM][K_DIM]);

/*******************************************************
 *    2D Convolution (kernel is type: double)
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K - 3x3 Convolution Kernel
 *
*******************************************************/
void conv2dGauss(pixel A[R_DIM][C_DIM], pixel C[R_DIM][C_DIM], float K[K_DIM][K_DIM]);

/********************************************************************
 *    Non-Maximum Suppression
 *    Inputs:  A - 120x160 Image Result Matrix
 *             G - 120x160 Gradient Magnitude Matrix
 *             T - 120x160 Gradient Phase Matrix
 *
**********************************************************************/
void nonMaxSuppression(pixel A[R_DIM][C_DIM], pixel G[R_DIM][C_DIM], float T[R_DIM][C_DIM]);

/********************************************************************
 *    Double Thresholding
 *    Inputs:  	  A 	 - 120X160 Image Matrix
 *             lowRatio  - low_thresh/high_thresh
 *             highRatio - high_thresh/maximum_value
 *
**********************************************************************/
void doubleThreshold (pixel A[R_DIM][C_DIM], float lowRatio, float highRatio);

/********************************************************************
 *    Edge Tracking by Hysteresis
 *    Inputs:  	 A 	  - 120x160 Image Array
 *             strong - Value of "strong" edge pixel = 255
 *              weak  - Value of "weak" edge pixel = 25
 *
**********************************************************************/
void edgeTracking(pixel A[R_DIM][C_DIM], pixel strong, pixel weak);

/********************************************************************
 *    Hypotenuse Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Result Matrix
 *
**********************************************************************/
void imgHypot (pixel X[R_DIM][C_DIM], pixel Y[R_DIM][C_DIM], pixel C[R_DIM][C_DIM]);

/********************************************************************
 *    Angle Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Result Matrix
 *
**********************************************************************/
void imgTheta (pixel X[R_DIM][C_DIM], pixel Y[R_DIM][C_DIM], float C[R_DIM][C_DIM]);

// Debugging functions for comparing rows of pixels
void printRowSum(pixel A[R_DIM][C_DIM]);
void printRowSumTheta(float A[R_DIM][C_DIM]);

/********************************************************************
 *    Converts Edge Map into Array of Vec2D structs
 *    Inputs:     E     - Edge map image
 *             edge_ind - array of x,y structs containing
 *                        edge point indicies
 * 
 *    Outputs: num_points - amount of pixels classified as edge points
 *
**********************************************************************/
uint16_t edge2Arr(pixel E[R_DIM][C_DIM], Vec2D edge_ind[NUM_PIX]);

/********************************************************************
 *    Prints out contents of edge points array
 *    Inputs:  edge_points  - array of Vec2D structures
 *
**********************************************************************/
void edgePrint(Vec2D edge_ind[], uint16_t size);

#endif

