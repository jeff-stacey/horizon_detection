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

#include <math.h>
#include "linalg.h"

void remove_barrel_distort_FO(Vec2D imdata[], int data_len, int im_width, int im_height, float pd)
{
    /*
    First order radial barrel distortion removing
    based on dp (distortion percentage)

    dp = (rd - rp)/rp
    rd => distance from centre to corner after distortion
    rp => distance from centre to corner before distortion
    */
   
    Vec2D corner_point;
    corner_point.x = im_width*0.5f;
    corner_point.y = im_height*0.5f;

    float r_sq = corner_point.x*corner_point.x + corner_point.y*corner_point.y;

    float k = pd / ((1.0f - pd) * r_sq) ;

    Vec2D point;
    rd = 0;

    for(int i=0; i<data_len; i++){

        point.x = imdata[i].x;
        point.y = imdata[i].y;

        r_sq = point.x*point.x + point.y*point.y;

        imdata[i].x *= 1.0f + k*r_sq;
        imdata[i].y *= 1.0f + k*r_sq;
    }
}

void remove_barrel_distort_KO(Vec2D imdata[], const int len_data, const float k_params[], const int k_len)
{
    /*
    K-th order barrel distortion
    k_params => array of k ordered parameters (k1,k2,k3...)

    Higher order parameters affect distortion considerable.
    Most likely, will only need to use 2 parameters
    each parameter will be << 1 (most likely on the order of 10^-6 or lower)

    Parameters must be calibrated for the camera
    */
    Vec2D temp_point;
    float rd;

    for(int i=0; i<len_data; i++){
        temp_point.x = imdata[i].x;
        temp_point.y = imdata[i].y;

        rd = norm(&temp_point);

        temp_point.x = 0;
        temp_point.y = 0;

        for(int j=1; j<=k_len; j++){
            temp_point.x += k_params[j-1]*powf(rd,2*j);
            temp_point.y += k_params[j-1]*powf(rd,2*j);
        }

        temp_point.x *= imdata[i].x;
        temp_point.y *= imdata[i].y;

        imdata[i].x += temp_point.x;
        imdata[i].y += temp_point.y;
    }
}
