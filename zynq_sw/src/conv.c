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
 *    Outputs: C - Modified Result Matrix
*******************************************************/

pixel conv2d(pixel A, pixel C, pixel K) {
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
            /*Minor Thresholding*/
            if (sum < 0) sum = 0;
            if (sum > max) sum = max;
            C[i][j] = sum;
        } 
    }
    return C;
};


/*******************************************************
 *    Seperable Convolve 2D
 *    Inputs:  A - 120x160 Image Matrix
 *             C - 120x160 Result Matrix
 *             K - 3x3 Seperable Convolution Kernel
 * 
 *    Outputs: C - Modified Result Matrix
*******************************************************/

pixel* sep_conv2d(pixel A[M][N], pixel K[k_dim][k_dim]) {

};
