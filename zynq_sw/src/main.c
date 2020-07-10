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
#include <stdint.h>
#include "conv.h"

#define IMG_ROWS 120
#define IMG_COLS 160
#define HELP_TEST 1 //Testing Trigger for helper functions

typedef int16_t pixel;

pixel TestImg[120][160];

int16_t kernel_x[3][3] = {
		//Using SOBEL kernel
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}
        };

int16_t kernel_y[3][3] = {
       //Using SOBEL kernel
            {1, 2, 1},
            {0, 0, 0},
            {-1, -2, -1}
        };
float result;


int main() {

    printf("Hello world\n\r");

    
    int column_acc = 0;

    for (int j = 0; j < IMG_COLS; j++)
    {
    	for (int i = 0; i < IMG_ROWS; i++)
    	{
    		column_acc += TestImg[i][j];
    	}
    	printf("column sum for %3d is %d\n", j, column_acc);
    }

    result = 3.1415926535;

    printf("result: %f\n", result);


    /* Helper Function Testing*/
    if (HELP_TEST)
    {
    	printf("Helper Function Testing Start\n\r");

    	/*Perform Helper Operations*/
    	printf("Applying functions to image\n");

    	pixel edge_x[120][160]; //Create x-direction gradient map output
    	pixel edge_y[120][160]; //Create y-direction gradient map output

    	conv2d(TestImg, edge_x, kernel_x); // Convolve
    	printf("x-direction 2D-convolution complete\n");

    	conv2d(TestImg, edge_y, kernel_y); // Convolve
    	printf("y-direction 2D-convolution complete\n");

    	pixel grad[120][160]; // Create Grad-Magnitude output
    	pixel theta[120][160]; // Create Grad-Direction output

    	img_hypot(edge_x, edge_y, grad);
    	printf("Combined x and y edge maps using img_hypot\n\r");
    	img_theta(edge_x, edge_y, theta);
    	printf("Obtained direction map using img_theta\n\r");

    }



asm volatile ("end_of_main:");
    return 0;
}
