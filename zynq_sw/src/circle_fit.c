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

#include "circle_fit.h"
#include "line.h"

#include <math.h>
#include <stdlib.h>

unsigned int nCr(unsigned int n, unsigned int r)
{
    if(r > n){
        return 0;
    }
    if(2*r > n){
        r = n-r;
    }
    if(r == 0){
        return 1;
    }

    unsigned int result = n;
    for(int i = 2; i <= r; ++i){
        result *= (n-i+1);
        result /= i;
    }
    return result;
}

void permutations2(Vec2D pairs[], int len_pairs, const Vec2D data[], int len_data)
{
    int pairs_index = 0;
    int n = 0;
    for(int i = 0; i < len_data; i++){
        for(int j = 0; j < len_data - n; j++){
            if(data[i].x != data[j+n].x && data[i].y != data[j+n].y){
                //pairs are represented as indices of data array
                Vec2D data_ind;
                data_ind.x = i;
                data_ind.y = j+n;
                pairs[pairs_index] = data_ind;
                pairs_index++;
            }
        }
        n++;
    }
}

void lineintersect_circle_fit(const Vec2D data[], int len_data, int sample_num, float result[3])
{
    //assuming data is subset of the overall data

    Vec2D p1;
    Vec2D p2;
    Vec2D p3;
    Vec2D p4;

    Vec2D v1;
    Vec2D v2;

    Vec2D m1;
    Vec2D m2;

    Line2D l1;
    Line2D l2;

    Vec2D intersect;
    Vec2D intersect_temp;
    int avg_n = 0;

    float vnorm;

    int n = 0;
    int m = 0;

    for(int i = 0; i < len_data; i++){
        n = 0;
        for(int j = 0; j < len_data - n*sample_num; j++){
            if(data[i*sample_num].x != data[(j+n)*sample_num].x && data[i*sample_num].y != data[(j+n)*sample_num].y){
                //pair of points at i and j+1  
                if(m%2 == 0){
                    p1.x = data[i*sample_num].x;
                    p1.y = data[i*sample_num].y;
                    p2.x = data[(j+n)*sample_num].x;
                    p2.y = data[(j+n)*sample_num].y;

                    v1.x = p2.x - p1.x;
                    v1.y = p2.y - p1.y;

                }else{
                    //printf("points:\n");
                    p3.x = data[(i+1)*sample_num%(sample_num*len_data)].x;
                    p3.y = data[(i+1)*sample_num%(sample_num*len_data)].y;
                    p4.x = data[(j+n)*sample_num].x;
                    p4.y = data[(j+n)*sample_num].y;

                    v2.x = p4.x - p3.x;
                    v2.y = p4.y - p3.y;

                    //printf("\n");
                    //printf("points used:\n");
                    //printf("(%f, %f) (%f, %f) and\n", p1.x, p1.y, p2.x, p2.y);
                    //printf("(%f, %f) (%f, %f)\n", p3.x, p3.y, p4.x, p4.y);


                    if(!(norm(&v1) < 10 || norm(&v2) < 10)){
                        //skip these points if theyre too close together
                        //mid point between p1 and p2

                        m1.x = 0.5*v1.x + p1.x;
                        m1.y = 0.5*v1.y + p1.y;
                        vnorm = norm(&v1);
                        v1.x = v1.x/vnorm;
                        v1.y = v1.y/vnorm;

                        m2.x = 0.5*v2.x + p3.x;
                        m2.y = 0.5*v2.y + p3.y;
                        vnorm = norm(&v2);
                        v2.x = v2.x/vnorm;
                        v2.y = v2.y/vnorm;

                        //line defined by u1 and m1
                        l1.a = v1.x; // m1.y - (m1.y + u1.y);
                        l1.b = v1.y; //m1.x - (m1.x + u1.x);
                        l1.c = -1*v1.x*m1.x - v1.y*m1.y; //m1.x*(m1.y + u1.y) - m1.y*(m1.x + u1.x);

                        //line defined by u2 and m2
                        l2.a = v2.x; //m2.y - (m2.y + u2.y);
                        l2.b = v2.y; //m2.x - (m2.x + u2.x);
                        l2.c = -1*v2.x*m2.x - v2.y*m2.y;//m2.x*(m2.y + u2.y) - m2.y*(m2.x + u2.x);

                        intersect_temp = find_intersection(&l1, &l2);
                        
                        //if lines are parallel
                        if(isnan(intersect_temp.x) || isnan(intersect_temp.y)){
                            //just skip this step
                        }else
                        {
                            //printf("found intersection at:\n");
                            //printf("(%f, %f) \n", intersect_temp.x, intersect_temp.y);
                            intersect.x += intersect_temp.x;
                            intersect.y += intersect_temp.y;

                            avg_n++;
                        }

                    }
                }
                m++;
                
            }
        }
        n++;
    }


    //printf("avg_n: %d\n", avg_n);
    intersect.x /= avg_n;
    intersect.y /= avg_n;

    Vec2D d;
    float r = 0;
    for(int i=0; i<len_data; i++){
        d.x = data[i].x - intersect.x;
        d.y = data[i].y - intersect.y;
        r += norm(&d);
    }
    //printf("len_data: %d\n", len_data);
    r /= len_data;

    result[0] = intersect.x;
    result[1] = intersect.y;
    result[2] = r;

}

void LScircle_fit(Vec2D data[], int len_data, float result[3])
{
    /*
    Implementation of linear least squares fitting of circle
    */

    float A[3][3] = {{0,0,0},{0,0,0},{0,0,0}};

    float B[3] = {0,0,0};

    for(int i=0; i<len_data; i++){
        // fill the matrix one point at a time
        A[0][0] += data[i].x*data[i].x;
        A[0][1] += data[i].x*data[i].y;
        A[0][2] += data[i].x;

        // the matrix is symmetric, so we can repeat some points
        A[1][0] = A[0][1];
        A[1][1] += data[i].y*data[i].y;
        A[1][2] += data[i].y;

        A[2][0] = A[0][2];
        A[2][1] = A[1][2];
        A[2][2] = len_data;

        float temp = data[i].x*data[i].x + data[i].y*data[i].y;
        B[0] += data[i].x*temp;
        B[1] += data[i].y*temp;
        B[2] += temp;
    }
    

    inv3(A);

    //result stored in array as (x_0,y_0,r)

    multiply33by31(A, B, result);

    result[0] = 0.5*result[0];
    result[1] = 0.5*result[1];
    result[2] = sqrt(result[2] + result[0]*result[0] + result[1]*result[1]);
}


void circleGOF(Vec2D data[], int len_data, float params[3], float result[], int calc_std)
{
    float mean_sq_error = 0;
    float mean_abs_error = 0;
    
    for(int i=0; i<len_data; i++){
        Vec2D d;
        d.x = data[i].x - params[0];
        d.y = data[i].y - params[1];

        float error = norm(&d) - params[2];

        mean_sq_error += error*error;
        mean_abs_error += fabsf(error);
    }

    mean_sq_error /= len_data;
    mean_abs_error/= len_data;

    if(calc_std){

        float std_sq_error = 0;
        float std_abs_error = 0;

        for(int i=0; i<len_data; i++){
            Vec2D d;
            d.x = data[i].x - params[0];
            d.y = data[i].y - params[1];

            float error = norm(&d) - params[2];
            
            std_sq_error += (error*error - mean_sq_error)*(error*error - mean_sq_error);
            std_abs_error += (fabsf(error) - mean_abs_error)*(fabsf(error) - mean_abs_error);
        }

        std_sq_error = sqrtf(std_sq_error/len_data);
        std_abs_error = sqrtf(std_abs_error/len_data);

        result[0] = mean_sq_error;
        result[1] = mean_abs_error;
        result[2] = std_sq_error;
        result[3] = std_abs_error;
    }else{
        result[0] = mean_sq_error;
        result[1] = mean_abs_error;
    }
    
}
