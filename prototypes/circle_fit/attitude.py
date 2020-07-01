#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun 20 19:01:09 2020

@author: byron
"""
import numpy as np
import matplotlib.pyplot as plt
import math

import itertools

from __main__ import *

def find_vert_closest(data):
    dist = 100
    p = 0
    for x in data:
        d = np.linalg.norm(np.zeros(2) - x)
        if d < dist:
            dist = d
            p = x
    
    return p

def find_attitude(r, c):
    #theta_z = roll
    #theta_x = pitch
    
    
    v1 = np.zeros(2) - c
    v1 = v1/np.linalg.norm(v1)
    v2 = np.array([0,1])
    theta_z = math.acos(np.clip(np.dot(v1, v2), -1.0, 1.0))
    
    
    #find sign of angle
    det = np.linalg.det([[v1[0],v2[0]],[v1[1],v2[1]]])
    
    if det < 0:
        theta_z = -1*theta_z
    
    vert = ((np.zeros(2) - c)/np.linalg.norm(np.zeros(2) - c))*r + c
    
    #check if centre is inside circle:
    if r > np.linalg.norm(np.zeros(2) - c):
        #the centre is inside the circel
        #k is negative
        k = -1*np.linalg.norm(vert-np.zeros(2))
    else:
        #centre is either on perimeter (k=0)
        #or outside circle (k is positive)
        k = np.linalg.norm(vert - np.zeros(2))
    
    d = 80
    
    theta_x = math.atan((k/d)*math.tan(FOV/2)) + math.asin(E_RADIUS/(E_RADIUS+ALTITUDE))
    return theta_x, theta_z


def find_nadir(r, c):
    th_x, th_z = find_attitude(r,c)
    th_x = -1*th_x
    th_z = -1*th_z
        
    nadir = np.matrix([0,0,-1])
        
    #rotate about x by th_x
    nadir = np.matrix([[1,              0, 0],
               [0, math.cos(th_x), -1*math.sin(th_x)],
               [0, math.sin(th_x), math.cos(th_x)]])*nadir.T
    
    #rotate about z by th_z
    nadir = np.matrix([[math.cos(th_z), -1*math.sin(th_z), 0],
                       [math.sin(th_z), math.cos(th_z), 0],
                       [0, 0, 1]])*nadir
    
    return nadir