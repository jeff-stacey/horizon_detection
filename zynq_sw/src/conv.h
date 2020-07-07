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

<<<<<<< HEAD

=======
>>>>>>> d3fda0f9068ea0ba57976872bcf57de0c51baeb8
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
#define pixel uint8_t //8-bit images, change if 14-bit

/*Image Dimensions*/
#define r_dim 120
#define c_dim 160
#define num_pix r_dim*c_dim

/*Kernel Dimensions*/
#define k_dim 3 //Unless otherwise specified, kernel is 3x3

/*Testing Trigger*/
#define TEST 1

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
void conv2d(pixel A[r_dim][c_dim], pixel C[r_dim][c_dim], pixel K[k_dim][k_dim]) {
    uint16_t rows = r_dim;
    uint16_t cols = c_dim;
    uint16_t a, b, i, j, sum;

    uint16_t max = 255; //Maximum pixel value

    /*Iterate through image*/
    for (i=0 ; 1< rows ; i++) {
        for (j=0 ; j < cols ; j++){
            sum = 0;
            /* Iterate through kernel */
            for (a=-1; a < 2; a++) {
                /*Check Row Indecies for out of bounds*/
                if (i+a < 0 || i+a > r_dim - 1) {
                    continue;
                } else {
                    for (b=-1; b < 2; b++) {
                        /*Check Column Indecies for out of bounds*/
                        if ( j+b < 0 || j+b > c_dim -1) {
                            continue;
                        } else {
                            /*Add to the sum*/
                            sum = sum + A[i+a][j+b]*K[a+1][b+1];
                        }
                    }
                }
            }
            /*Sum Thresholding*/
            if (sum < 0) sum = 0;
            if (sum > max) sum = max;
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
void conv1d(pixel A[r_dim][c_dim], pixel K_r[k_dim], pixel K_c[k_dim], pixel C[r_dim][c_dim]) {
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
void img_hypot (pixel X[r_dim][c_dim], pixel Y[r_dim][c_dim], pixel C[r_dim][c_dim]) {

    uint16_t i, j;
    double result;

    for (i=0; i < r_dim-1 ; i++) {
        for (j=0; j < c_dim-1 ; j++) {
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
void img_theta (pixel X[r_dim][c_dim], pixel Y[r_dim][c_dim], pixel C[r_dim][c_dim]) {

    short i, j;
    double result;

    for (i=0; i < r_dim-1 ; i++) {
        for (j=0; j < c_dim-1 ; j++) {
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

void edge2bin(pixel E[r_dim][c_dim], pixel B[r_dim][c_dim], short u_t) {

    short i, j;

    for (i=0; i < r_dim-1 ; i++) {
        for (j=0; j < c_dim-1 ; j++) {
            if (E[i][j] < u_t) {
                B[i][j] = 0;
            } else {
                B[i][j] = 1;
            }
        };
    };
};

#endif
