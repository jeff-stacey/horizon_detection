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
    corner_point.x = im_width/2;
    corner_point.y = im_height/2;

    float rd = norm(&corner_point);

    Vec2D corner_point_p;
    corner_point_p.x = corner_point.x/(pd+1);
    corner_point_p.y = corner_point.y/(pd+1);

    float k = (corner_point.x - corner_point_p.x)/(corner_point.x*(rd*rd));

    Vec2D temp_point;

    for(int i=0; i<data_len; i++){
        temp_point.x = imdata[i].x;
        temp_point.y = imdata[i].y;

        imdata[i].x = temp_point.x + temp_point.y*(k*rd*rd);
        imdata[i].y = temp_point.y + temp_point.y*(k*rd*rd);
    }
}