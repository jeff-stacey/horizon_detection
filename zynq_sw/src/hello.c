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
#include "xil_printf.h"
#include "conv.h"

#define TEST 1
#define DEBUG

//TODO
// The image will be here. The attribute places it in a special section.
// This array's starting address is 0x0011000 as specified in the linker file.
pixel __attribute__((section (".imageData"))) TestImg[120][160];

int main() {

    print("Hello world\n\r");

        if (TEST) {
        printf("Helper Function Testing Start\n\r");

        /*Perform Helper Operations*/
        printf("Applying functions to image\n\r");
        //Create x-direction gradient map output
        pixel edge_x[120][160];               
        pixel kernel_x[3][3] = {
            //Using SOBEL kernel
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}
        };
        //Create y-direction gradient map output
        pixel edge_y[120][160];
        pixel kernel_y[3][3] = {
            //Using SOBEL kernel
            {1, 2, 1},
            {0, 0, 0},
            {-1, -2, -1}
        };

        // Perform convolutions
        conv2d(TestImg, edge_x, kernel_x);
        printf("x-direction 2D-convolution complete\n\r");
        conv2d(TestImg, edge_y, kernel_y);
        printf("y-direction 2D-convolution complete\n\r");

        //Combine images to obtain edge map and direction map
        pixel grad[120][160];
        pixel theta[120][160];
        img_hypot(edge_x, edge_y, grad);
        printf("Combined x and y edge maps using img_hypot\n\r");
        img_theta(edge_x, edge_y, theta);
        printf("Obtained direction map using img_theta\n\r");


        /*Save results into files for comparison*/
        print("Saving results to file\n\r");


    }

    return 0;
}
