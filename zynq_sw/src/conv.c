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
#define M 120
#define N 160


/*******************
 *    FUNCTIONS    *
********************/

/*******************************************************
 *    Convolve 2D
 *    Inputs:  A - 120x160 Image Matrix
 *             K - 3x3 Convolution Kernel
 * 
 *    Outputs: c - Pointer to resulting C matrix
*******************************************************/

pixel* conv2d(pixel A, pixel K) {
  static pixel C[M][N];



};


/*******************************************************
 *    Seperable Convolve 2D
 *    Inputs:  A - 120x160 Image Matrix
 *             K - 3x3 Seperable Convolution Kernel
 * 
 *    Outputs: c - Pointer to resulting C matrix
*******************************************************/

pixel* sep_conv2d(pixel A, pixel K) {

};



int main() {

    print("Hello world\n\r");

    return 0;
}