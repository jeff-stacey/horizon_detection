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

#include <stdio.h>
#include <stdint.h>
#include "math.h"

/*TODO: HELPER FUNCTIONS FOR EDGE DETECTION
    - DONE 2d convolve (may have scaling issues in output around edges)
    - 1d convolve with image
    - read image into array?
    - grey edge map to binary edge map conversion
    - DONE element-wise array "hypotenuse" operation
    - DONE element-wise array "theta" operation
    - Non-Max suppression (rquires edge map and theta matrix)
    - Double Thresholding
    - Edge Tracking by Hysterysis
*/

/*******************
 *    DEFINES      *
********************/

/* Types */
typedef int16_t pixel;  //8-bit images, change if 14-bit

/*Image Dimensions*/
#define R_DIM 120
#define C_DIM 160
#define NUM_PIX R_DIM*C_DIM

/*Kernel Dimensions*/
#define K_DIM 3 //Unless otherwise specified, kernel is 3x3


/*******************
 *    FUNCTIONS    *
********************/

/*******************************************************
 *    2D Convolution
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K - 3x3 Convolution Kernel
 *
*******************************************************/
void conv2d(pixel A[R_DIM][C_DIM], pixel C[R_DIM][C_DIM], int16_t K[K_DIM][K_DIM]) {
    int16_t rows = R_DIM;
    int16_t cols = C_DIM;
    int16_t a, b, i, j;
    volatile int16_t sum = 0;

    /*Iterate through image*/
    for (i=1 ; i < rows-1 ; i++) {
        for (j=1 ; j < cols-1 ; j++){
            sum = 0;
            /* Iterate through kernel */
            for (a=-1; a < 2; a++) {
                for (b=-1; b < 2; b++) {
                    /*Add to the sum*/
                    sum += A[i+a][j+b]*K[a+1][b+1];
                }
            }
            /*Sum Thresholding*/
            if (sum < 0) sum = 0;
            if (sum > 255) sum = 255;
            C[i][j] = sum;
        }
    }
};

/********************************************************************
 *    1D Convolution
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K_c - 3x1 Seperated Section of Kernel (column vector)
 *             K_r - 1x3 Seperated Section of Kernel (row vector)
 *
**********************************************************************/
void conv1d(pixel A[R_DIM][C_DIM], pixel K_r[K_DIM], pixel K_c[K_DIM], pixel C[R_DIM][C_DIM]) {
    //TODO
};

/********************************************************************
 *    Hyptotenuse Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Result Matrix
 *             r - # of rows in matricies
 *             c - # of columns in matricies
 *
**********************************************************************/
void img_hypot (pixel X[R_DIM][C_DIM], pixel Y[R_DIM][C_DIM], pixel C[R_DIM][C_DIM]) {

    uint16_t i, j;
    double result;

    for (i=0; i < R_DIM-1 ; i++) {
        for (j=0; j < C_DIM-1 ; j++) {
            result =  hypot((double)X[i][j], (double)Y[i][j]);
            C[i][j] = (pixel)result;
        };
    };
};

/********************************************************************
 *    Angle Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Result Matrix
 *             r - # of rows in matricies
 *             c - # of columns in matricies
 *
**********************************************************************/
void img_theta (pixel X[R_DIM][C_DIM], pixel Y[R_DIM][C_DIM], pixel C[R_DIM][C_DIM]) {

    short i, j;
    double result;

    for (i=0; i < R_DIM-1 ; i++) {
        for (j=0; j < C_DIM-1 ; j++) {
            result =  atan((double)Y[i][j]/(double)X[i][j]);
            C[i][j] = (pixel)result;
        };
    };
};

/********************************************************************
 *    Edge Map to Binary Edge Map Conversion (element-wise)
 *    Inputs:  E   - Edge map image
 *             B   - Result Matrix
 *             r   - # of rows in matricies
 *             c   - # of columns in matricies
 *             u_t - Upper threshold value
 *
**********************************************************************/

void edge2bin(pixel E[R_DIM][C_DIM], pixel B[R_DIM][C_DIM], short u_t) {

    short i, j;

    for (i=0; i < R_DIM-1 ; i++) {
        for (j=0; j < C_DIM-1 ; j++) {
            if (E[i][j] < u_t) {
                B[i][j] = 0;
            } else {
                B[i][j] = 1;
            }
        };
    };
};

#endif

