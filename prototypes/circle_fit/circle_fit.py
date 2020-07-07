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
        

def choose_samples(data):
    points = []
    for i in range(len(data)):
        if i%5 == 0:
            points.append(data[i])
        
    pairs = itertools.permutations(points,2)
    pairs = list(pairs)
    np.random.shuffle(pairs)
    
    return pairs

def circle_find(data):
    #find data point pairs

    pairs = choose_samples(data)
    
    n = 0
    avg = np.zeros(2)
    for i in range(len(pairs)-1):
        p1 = pairs[i]
        p2 = pairs[i+1]
        
        v1 = (p1[1]-p1[0])
        v2 = (p2[1]-p2[0])
        
        if np.linalg.norm(v1) < 5 or np.linalg.norm(v2) < 5:
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