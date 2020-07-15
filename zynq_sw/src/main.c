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

#include "linalg.h"
#include "circle_fit.h"
#include "attitude.h"

#define IMG_ROWS 120
#define IMG_COLS 160
#define EDGE_TEST 1 //Testing Trigger for edge_detection

typedef int16_t pixel;

/********************************
 *			Global Vars			*
 ********************************/
pixel TestImg[120][160];

double kernel_gauss[3][3] = {
		// Gaussian Blur kernel
			{1/16, 2/16, 1/16},
			{2/16, 4/16, 2/16},
			{1/16, 2/16, 1/16}
		};

int16_t kernel_x[3][3] = {
		// SOBEL kernel
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}
        };

int16_t kernel_y[3][3] = {
        // SOBEL kernel
            {1, 2, 1},
            {0, 0, 0},
            {-1, -2, -1}
        };

pixel blurred[120][160];	// Create gaussian blurring step output
pixel edge_x[120][160]; 	// Create x-direction gradient map output
pixel edge_y[120][160]; 	// Create y-direction gradient map output
pixel grad[120][160]; 		// Create Grad-Magnitude output
double theta[120][160]; 	// Create Grad-Direction output
pixel suppressed[120][160]; // Create Output for non-max suppression step

float lowRatio = 0.05;
float highRatio = 0.09;
pixel strong = 255;
pixel weak = 25;

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


    /* Edge Detection Testing*/
    if (EDGE_TEST)
    {
    	printf("\nEdge Detection Testing Start\n");

    	printf("\nStep 1: Obtain gradient magnitude and phase maps\n");

    	conv2dGauss(TestImg, blurred, kernel_gauss);
    	printf("\tGaussian blurring of test image complete\n");

    	conv2d(blurred, edge_x, kernel_x);
    	printf("\tx-direction 2D-convolution complete\n");

    	conv2d(blurred, edge_y, kernel_y);
    	printf("\ty-direction 2D-convolution complete\n");

    	imgHypot(edge_x, edge_y, grad);
    	printf("\tObtained gradient magnitude map\n");

    	imgTheta(edge_x, edge_y, theta);
    	printf("\tObtained gradient phase map\n\r");

    	printf("Step 2: Perform non-maximum suppression\n");

    	nonMaxSuppression(suppressed, grad, theta);
    	printf("\tNon-Max suppression complete\n\r");

    	printf("Step 3: Perform Thresholding and Edge Tracking\n");

    	doubleThreshold(suppressed, lowRatio, highRatio);
    	printf("\tDouble Thresholding complete\n");

    	edgeTracking(suppressed, strong, weak);
    	printf("\tEdge Tracking complete\n\r");

    	printf("Edge Detection Test Complete\n");
    }



asm volatile ("end_of_main:");
    return 0;
}
