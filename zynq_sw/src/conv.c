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

#include <stdio.h>
#include <math.h>

/*TODO: HELPER FUNCTIONS FOR EDGE DETECTION
    -2d convolve DONE (may have scaling issues in output around edges)
    -seperable 2d convolve
    -1d convolve
    - read image?
    - grey edge map to binary edge map conversion
    - element-wise array "hypotenuse" operation
    - element-wise array "theta" operation
    - Non-Max suppression (rquires edge map and theta matrix)
    - Double Thresholding
    - Edge Tracking by Hysterysis
*/

/*******************
 *    DEFINES      *
********************/

/* Types */
#define pixel char //8-bit images, change if 14-bit

/*Image Dimensions*/
#define r_dim 120
#define c_dim 160

/*Kernel Dimensions*/
#define k_dim 3 //Unless otherwise specified, kernel is 3x3

/*******************
 *    FUNCTIONS    *
********************/

/*******************************************************
 *    Convolve 2D
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K - 3x3 Convolution Kernel
 *
*******************************************************/
void conv2d(pixel A[r_dim][c_dim], pixel C[r_dim][c_dim], pixel K[k_dim][k_dim]) {
    short rows = r_dim;
    short cols = c_dim;
    short a, b, i, j, k, sum;

    short max = 255; //Maximum pixel value

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
 *    Seperable Convolve 2D
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K_c - 3x1 Seperated Section of Kernel (column vector)
 *             K_r - 1x3 Seperated Section of Kernel (row vector)
 * 
**********************************************************************/
void sep_conv2d(pixel A[r_dim][c_dim], pixel K_r[k_dim], pixel K_c[k_dim], pixel C[r_dim][c_dim]) {
    //TODO
};

/********************************************************************
 *    Hyptotenuse Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Float-type Result Matrix
 *             r - # of rows in matricies
 *             c - # of columns in matricies
 * 
**********************************************************************/
void img_hypot (pixel X[], pixel Y[], pixel C[], short r, short c) {

    short i, j;
    double result;

    for (i=0; i < r-1 ; i++) {
        for (j=0; j < c-1 ; j++) {
            result =  hypot((double)X[i][j], (double)Y[i][j]);
            C[i][j] = (pixel)result;
        };
    };
};

/********************************************************************
 *    Angle Calculation (element-wise)
 *    Inputs:  X - First array operand
 *             Y - Second array operand
 *             C - Float-type Result Matrix
 *             r - # of rows in matricies
 *             c - # of columns in matricies
 * 
**********************************************************************/
void img_theta (pixel X[], pixel Y[], pixel C[], short r, short c) {

    short i, j;
    double result;

    for (i=0; i < r-1 ; i++) {
        for (j=0; j < c-1 ; j++) {
            result =  atan((double)Y[i][j]/(double)X[i][j]);
            C[i][j] = (pixel)result;
        };
    };
};

int main() {

    print("Hello world\n\r");

    return 0;
}