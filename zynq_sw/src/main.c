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
#include "perf.h"

#include <stdint.h>
#include <math.h>

/********************************
 *            Global Vars            *
 ********************************/

// INPUTS: should be populated externally
// e.g. by the test script or by hardware reading sensors

// Input image
pixel TestImg[120][160];

// Magnetometer readings and transformation
int16_t magnetometer_reading[3];
float magnetometer_transformation[16];

// Altitude (found externally)
float altitude;

// PARAMETERS: are initialized, but can be modified externally
// they should remain constant while main is running

// We have a couple algorithm options. 
// This variable selects which one to run:
// 0 : edge detection and least-squares curve fitting
// 1 : edge detection and chord curve fitting
int alg_choice = 0;

// If the edge detection only finds a few points, they're likely noise or the
// horizon is barely visible - fitting to these points can give wildly
// inaccurate results. Below this threshold, we don't estimate an orientation.
int min_required_points = 10;

// If the radius of the circle we fit is smaller than it should be, something
// has probably gone wrong - maybe we're fitting to noise, maybe we're fitting
// to a tiny part of the horizon on the side of the image. Either way the
// result is probably wrong.
float min_circle_radius = 150.0;

// Edge detection parameters
float lowRatio = 0.5;
float highRatio = 0.8;
pixel strong = 0x3fff;  // Totally black pixel == 16383 == 0x3fff
                        // Totally white pixel == 0
pixel weak = 0x666;     // set weak to ~10% of total magnitude

// INTERMEDIATE PRODUCTS: used by the algorithm for temporary storage

// Edge Detection intermediate products
pixel blurred[120][160];    // Create gaussian blurring step output
pixel edge_x[120][160];     // Create x-direction gradient map output
pixel edge_y[120][160];     // Create y-direction gradient map output
pixel grad[120][160];         // Create Grad-Magnitude output
float theta[120][160];         // Create Grad-Direction output
pixel suppressed[120][160]; // Create Output for non-max suppression step
uint16_t num_points = 0;
Vec2D edge_points[19200];   // Create output array for number of edges

// Circle fitting intermediate products
// array containing (x_0, y_0, r) circle parameters
float circ_params[3];
// the rest of these are specifically for chord fitting
int subset_num = 20;

// RESULTS:

// indicates if the output is valid, and if not, why it's invalid
// 0: valid nadir vector
// 1: edge detection didn't find enough points (see min_required_points parameter)
// 2: fit circle radius was too small (see min_circle_radius parameter)
int reject;

float nadir[3];
Quaternion orientation;
uint32_t cycles = 0;
float mean_sq_error;
float mean_abs_error;

int main() {

    init_ccount();

    // there's some overhead to the cycle count call
    uint32_t overhead = get_ccount();
    // it should be fixed, so if we measure it, we can subtract it off
    overhead = get_ccount() - overhead;

    uint32_t t0 = get_ccount();
    
    dprintf("Starting horizon detection.\n\r");

    for (int i = 0; i < 3; i++){
        dprintf("%d\n", magnetometer_reading[i]);
    }

    dprintf("\nEdge Detection Testing Start\n");
    //printRowSum(TestImg);

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

    // Current Method of storing Edge_Points
    num_points = edge2Arr(suppressed,edge_points);
    dprintf("\tEdges Stored in \"edge_points\" array\n\r");
    dprintf("\tEdge detection found %d points\n", num_points);
    //edgePrint(edge_points,num_points);

    dprintf("Edge Detection Complete\n");
    
    if (num_points > min_required_points) {
        // if the edge detection returned enough points, we can proceed to curve fitting
        
        if (alg_choice == 0){
            //least-squares fit
            dprintf("Starting least-squares fit\n");
            LScircle_fit(edge_points, num_points, circ_params);
        } else if (alg_choice == 1) {
            //chord fitting
            dprintf("Starting Chord fit\n");
            
            dprintf("Fitting curve\n");
            int num_samples = ceil(num_points/subset_num);
            lineintersect_circle_fit(edge_points, num_samples, subset_num, circ_params);
        }

        if (circ_params[2] > min_circle_radius) {
            // Magnetometer reading in homogenous coordinates
            float mag_float[3] = {(float)magnetometer_reading[0], (float)magnetometer_reading[1], (float)magnetometer_reading[2]};
            
            // The magnetometer transformation is given as an affine matrix, but only the rotation is needed.
            float mag_rotation[3][3] = {
                {magnetometer_transformation[0], magnetometer_transformation[1], magnetometer_transformation[2]},
                {magnetometer_transformation[4], magnetometer_transformation[5], magnetometer_transformation[6]},
                {magnetometer_transformation[8], magnetometer_transformation[9], magnetometer_transformation[10]}
            };
            multiply33by31(mag_rotation, mag_float, mag_float);

            dprintf("Computing nadir vector\n");
            find_nadir(circ_params, altitude, nadir, mag_float, &orientation);
            reject = 0;
        } else {
            dprintf("Circle radius below threshold - result invalid\n");
            reject = 2;
        }


        // Goodness-of-fit checking not needed.
        //float errors[2];
        //dprintf("Checking goodness-of-fit\n");
        //circleGOF(edge_points, num_points, circ_params, errors, 0);
        //mean_sq_error = errors[0];
        //mean_abs_error = errors[1];
        //dprintf("errors: %f, %f\n", mean_sq_error, mean_abs_error);
    } else {
        // if the edge detection doesn't return any points, we probably can't see the horizon
        dprintf("Not enough points - skipping fit\n");
        reject = 1;
    }


    if (reject != 0) {
        // we don't have a valid nadir vector to return
        nadir[0] = 0.0;
        nadir[1] = 0.0;
        nadir[2] = 0.0;
        orientation.w = 0.0;
        orientation.x = 0.0;
        orientation.y = 0.0;
        orientation.z = 0.0;
    }


    uint32_t t1 = get_ccount();
    cycles = t1 - t0 - overhead;
    dprintf("took %lu cycles\n", cycles);

    //print results
    dprintf("nadir:\n");
    print3(nadir);

    asm volatile ("end_of_main:");
    return 0;
}
