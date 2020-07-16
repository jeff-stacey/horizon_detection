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

#include "common.h"
#include "edge.h"
#include "linalg.h"
#include "circle_fit.h"
#include "attitude.h"

#include <stdint.h>

typedef int16_t pixel;

/********************************
 *			Global Vars			*
 ********************************/

// Input image
pixel TestImg[120][160];

// Edge Detection intermediate products
pixel blurred[120][160];	// Create gaussian blurring step output
pixel edge_x[120][160]; 	// Create x-direction gradient map output
pixel edge_y[120][160]; 	// Create y-direction gradient map output
pixel grad[120][160]; 		// Create Grad-Magnitude output
double theta[120][160]; 	// Create Grad-Direction output
pixel suppressed[120][160]; // Create Output for non-max suppression step

// Edge detection parameters
float lowRatio = 0.3;
float highRatio = 0.67;
pixel strong = 255;
pixel weak = 25;

float result;

int main() {

    dprintf("Starting horizon detection.\n\r");

    dprintf("\nEdge Detection Testing Start\n");

    conv2dGauss(TestImg, blurred, kernel_gauss);
    dprintf("\tGaussian blurring of test image complete\n");
    printRowSum(blurred);

    conv2d(blurred, edge_x, kernel_x);
    dprintf("\tx-direction 2D-convolution complete\n");
    printRowSum(edge_x);

    conv2d(blurred, edge_y, kernel_y);
    dprintf("\ty-direction 2D-convolution complete\n");
    printRowSum(edge_y);


    imgHypot(edge_x, edge_y, grad);
    dprintf("\tObtained gradient magnitude map\n");
    printRowSum(grad);

    imgTheta(edge_x, edge_y, theta);
    dprintf("\tObtained gradient phase map\n\r");
    printRowSumTheta(theta);

    imgTheta(edge_x, edge_y, theta);
    dprintf("\tObtained gradient phase map\n\r");

    nonMaxSuppression(suppressed, grad, theta);
    dprintf("\tNon-Max suppression complete\n\r");
    //printRowSum(suppressed);

    doubleThreshold(suppressed, lowRatio, highRatio);
    dprintf("\tDouble Thresholding complete\n");
    //printRowSum(suppressed);

    edgeTracking(suppressed, strong, weak);
    dprintf("\tEdge Tracking complete\n\r");
    //printRowSum(suppressed);

    dprintf("Edge Detection Test Complete\n");
    //printRowSum(suppressed);

    asm volatile ("end_of_main:");
    return 0;
}
