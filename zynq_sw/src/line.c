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

#include "line.h"
#include "common.h"

#include <math.h>

Vec2D find_intersection(Line2D* L1, Line2D* L2){
    //3d ordered triple representing the intersection
    float P[3];
    P[0] = L1->b*L2->c - L2->b*L1->c;
    P[1] = L2->a*L1->c - L1->a*L2->c;
    P[2] = L1->a*L2->b - L2->a*L1->b;

    Vec2D intersection;

    if(P[2] == 0){
        //lines do not intersect
    	intersection.x = NAN;
    	intersection.y = NAN;
        return intersection;
    }else{
        //Project the 3d triple onto 2D to get the intersection
        intersection.x = P[0]/P[2];
        intersection.y = P[1]/P[2];
        return intersection;
    }
} 

void printline(Line2D* L)
{
	dprintf("%fx + %fy + %f = 0\n", L->a, L->b, L->c);
}
