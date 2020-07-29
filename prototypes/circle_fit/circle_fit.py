#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun 20 19:00:56 2020

@author: byron
"""


import numpy as np
import matplotlib.pyplot as plt
import math

import itertools

from __main__ import *


class Line:
    def __init__(self, a,b,c):
        
        self.a = a
        self.b = b
        self.c = c
        
    def plot(self, a=-80, b=80):
        x = np.linspace(a,b,10)
        y = (self.a*x+self.c)/self.b
        plt.grid(True)
        plt.plot(x,y)
        
    def find_intersection(self,L):
        #do some whack shit to find the intersection of 2 lines
        P = (self.b*L.c-L.b*self.c, L.a*self.c-self.a*L.c, self.a*L.b-L.a*self.b)
        
        if P[2] == 0:
            #lines do not intersect
            return None
        else:
            return np.array([P[0]/P[2],-P[1]/P[2]])
        

def permutations2(A):
    "some array A"
    pairs = []
    
    n = 0
    for i in range(len(A)):
        value1 = A[i]
        for j in range(len(A)-n):
            value2 = A[j+n]
            if not np.all(value1 == value2):
                pairs.append((value1,value2))
        n += 1
        
    return pairs

def choose_samples(data):
    points = []
    for i in range(len(data)):
        if i%1 == 0:
            points.append(data[i])
            
    points = np.array(points)
            
# =============================================================================
#     plt.grid()
#     plt.gca().set_aspect("equal")
#     plt.xlim(-300, 300)
#     plt.ylim(-300, 300)
#     plt.scatter(points[:,0], points[:,1])
# =============================================================================
    
    pairs = permutations2(points)
# =============================================================================
#     pairs = itertools.permutations(points,2)
#     pairs = list(pairs)
#     np.random.shuffle(pairs)
# =============================================================================
    
    return pairs

def circle_find(data):
    #find data point pairs

    pairs = choose_samples(data)
    
    print("%d data points" % len(data))
    print("%d sampled points" % len(data))
    
    n = 0
    avg = np.zeros(2)
    for i in range(len(pairs)-1):
        p1 = pairs[i]
        p2 = pairs[i+1]
        
        v1 = (p1[1]-p1[0])
        v2 = (p2[1]-p2[0])
        
        if np.linalg.norm(v1) < 2 or np.linalg.norm(v2) < 2:
            continue
        
        m1 = 0.5*v1+p1[0]
        u1 = np.array([v1[1], -1*v1[0]])
        m12 = m1 + u1
        
        m2 = 0.5*v2+p2[0]
        u2 = np.array([v2[1], -1*v2[0]])
        m22 = m2 + u2
           
        l1 = Line(m1[1]-m12[1], (m1[0]-m12[0]), m1[0]*m12[1]-m1[1]*m12[0])
        l2 = Line(m2[1]-m22[1], (m2[0]-m22[0]), m2[0]*m22[1]-m2[1]*m22[0])

        intersection = l1.find_intersection(l2)
        if type(intersection) == type(None):
            pass
        else:
            avg += intersection
            n = n + 1
    
    centre = avg/n
    
    #find average radius
    r = 0
    for x in data:
        r += np.linalg.norm(x-centre)
    r = r/len(data)
    
    return centre, r

def LSfit(data):
    A = np.zeros((3,3))
    
    A[0,0] = np.sum(data[:,0]**2)
    A[0,1] = np.sum(data[:,0]*data[:,1])
    A[0,2] = np.sum(data[:,0])
    
    A[1,0] = np.sum(data[:,0]*data[:,1])
    A[1,1] = np.sum(data[:,1]**2)
    A[1,2] = np.sum(data[:,1])
    
    A[2,0] = np.sum(data[:,0])
    A[2,1] = np.sum(data[:,1])
    A[2,2] = len(data[:,1])
    
    B = np.zeros((3,1))
    
    B[0] = np.sum(data[:,0]*(data[:,0]**2 + data[:,1]**2))
    B[1] = np.sum(data[:,1]*(data[:,0]**2 + data[:,1]**2))
    B[2] = np.sum(data[:,0]**2 + data[:,1]**2)
    
    print(A)
    print(B)
        
    ans = np.linalg.inv(np.matrix(A))*np.matrix(B)
    ans = np.array(ans.T)[0]
    
    print(np.linalg.inv(A))
    
    centre = np.array([0.5*ans[0], 0.5*ans[1]])
    r = np.sqrt(ans[2] + (0.5*ans[0])**2 + (0.5*ans[1])**2)
    
    return centre, r



def circleGOF(data, params, calc_std=False):
    
    mean_sq_error = 0
    mean_abs_error = 0
    
    centre = np.array([params[0], params[1]])
    r = params[2]
    
    for i in range(len(data)):
        
        x = data[i]
        
        error = (np.linalg.norm(x-centre) - r)
        
        mean_sq_error += error**2
        mean_abs_error += abs(error)
    
    mean_sq_error /= len(data)
    mean_abs_error /= len(data)
        
    if calc_std:
        
        std_sq_error = 0
        std_abs_error = 0
        
        for i in range(len(data)):
            
            x = data[i]
        
            error = (np.linalg.norm(x-centre) - r)
            
            std_sq_error += (error**2 - mean_sq_error)**2
            std_abs_error += (abs(error) - mean_sq_error)**2
            
        std_sq_error = np.sqrt(std_sq_error/len(data))
        std_abs_error = np.sqrt(std_abs_error/len(data))
        
        return mean_sq_error, std_sq_error, mean_abs_error, std_abs_error
    
    else:
        
        return mean_sq_error, mean_abs_error
        
        
        
        