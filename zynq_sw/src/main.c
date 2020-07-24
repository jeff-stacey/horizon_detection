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
#include <math.h>

/********************************
 *			Global Vars			*
 ********************************/

// Input image
pixel TestImg[120][160];

// We have a couple algorithm options. 
// This variable selects which one to run:
// 0 : edge detection and least-squares curve fitting
// 1 : edge detection and chord curve fitting
// 2 : vsearch (not yet implemented)
int alg_choice = 1;

// Edge detection parameters
float lowRatio = 0.3;
float highRatio = 0.67;
pixel strong = 0x3fff;  // Totally black pixel == 16383 == 0x3fff
                        // Totally white pixel == 0
pixel weak = 0x666;     // set weak to ~10% of total magnitude

// Edge Detection intermediate products
pixel blurred[120][160];	// Create gaussian blurring step output
pixel edge_x[120][160]; 	// Create x-direction gradient map output
pixel edge_y[120][160]; 	// Create y-direction gradient map output
pixel grad[120][160]; 		// Create Grad-Magnitude output
float theta[120][160]; 	    // Create Grad-Direction output
pixel suppressed[120][160]; // Create Output for non-max suppression step
uint16_t num_points = 0;
Vec2D edge_points[19200];   // Create output array for number of edges

// Circle fitting intermediate products
// array containing (x_0, y_0, r) circle parameters
float circ_params[3];
// the rest of these are specifically for chord fitting
int subset_num = 20;

// Results
float nadir[3];

int main() {

    dprintf("Starting horizon detection.\n\r");

    if ((alg_choice == 0) || (alg_choice == 1)) {
        dprintf("\nEdge Detection Testing Start\n");
        printRowSum(TestImg);

        dprintf("\tInitialized all output arrays\n");

        conv2dGauss(TestImg, blurred, kernel_gauss);
        dprintf("\tGaussian blurring of test image complete\n");
        //printRowSum(blurred);

        conv2d(blurred, edge_x, kernel_x);
        dprintf("\tx-direction 2D-convolution complete\n");
        //printRowSum(edge_x);

        conv2d(blurred, edge_y, kernel_y);
        dprintf("\ty-direction 2D-convolution complete\n");
        //printRowSum(edge_y);

        imgHypot(edge_x, edge_y, grad);
        dprintf("\tObtained gradient magnitude map\n");
        //printRowSum(grad);

        imgTheta(edge_x, edge_y, theta);
        dprintf("\tObtained gradient phase map\n\r");
        //printRowSumTheta(theta);

        nonMaxSuppression(suppressed, grad, theta);
        dprintf("\tNon-Max suppression complete\n\r");
        //printRowSum(suppressed);

        doubleThreshold(suppressed, lowRatio, highRatio);
        dprintf("\tDouble Thresholding complete\n");
        //printRowSum(suppressed);

        edgeTracking(suppressed, strong, weak);
        dprintf("\tEdge Tracking complete\n\r");
        //printRowSum(suppressed);

        // Current Method of storing Edge_Points (could speed up with malloc?) 
        num_points = edge2Arr(suppressed,edge_points);
        dprintf("\tEdges Stored in \"edge_points\" array\n\r");
        edgePrint(edge_points,num_points);

        dprintf("Edge Detection Complete\n");

        if (alg_choice == 0){
            //least-squares fit
            dprintf("Starting least-squares fit\n");
            LScircle_fit(edge_points, num_points, circ_params);
        } else if (alg_choice == 1) {
            //chord fitting
            dprintf("Starting chord fit\n");
            
            dprintf("Fitting curve\n");
            int num_samples = ceil(num_points/subset_num);
            lineintersect_circle_fit(edge_points, num_samples, subset_num, circ_params);
        }

        find_nadir(circ_params, nadir);
    }




    //print results
    printf("nadir:\n");
    print3(nadir);

    asm volatile ("end_of_main:");
    return 0;
}
