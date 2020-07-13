#include <stdio.h>
#include <stdint.h>
#include "math.h"


typedef struct
{
    /*
    Line implicitly defined as:

    ax + by + c = 0
    */
    float a;
    float b;
    float c;

} Line2D;



Vec2D find_intersection(Line2D* L1, Line2D* L2){
    //3d ordered triple representing the intersection
    float P[3];
    P[0] = L1->b*L2->c - L2->b*L1->c;
    P[1] = L2->a*L1->c - L1->a*L2->c;
    P[2] = L1->a*L2->b - L2->a*L1->b;

    Vec2D intersection;

    if(P[2] == 0){
        //lines do not intersect
    	intersection.x = INFINITY;
    	intersection.y = INFINITY;
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
	printf("%fx + %fy + %f = 0\n", L->a, L->b, L->c);
}

