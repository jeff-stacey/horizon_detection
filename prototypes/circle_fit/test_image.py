#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Jun 20 19:04:14 2020

@author: byron
"""


import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import math

import os
import struct

import itertools

import circle_fit
import attitude
import edge_detection

ALTITUDE = 500
E_RADIUS = 6378.136
FOV = 57*(math.pi/180)

image_folder = '/home/byron/Documents/horizon_detection/prototypes/test_images/images'
data_folder = '/home/byron/Documents/horizon_detection/prototypes/test_images/metadata'

images = np.sort(os.listdir(image_folder))
metadata = np.sort(os.listdir(data_folder))

test_results = []

for im, da in zip(images, metadata):
    print(im)
    with open(data_folder+'/'+da,'rb') as f:
        chunk = f.read(44)
    
    m = struct.unpack('ffiifffffff',chunk)
    
    metadata = {'altitude':m[0],
                'fov_h':m[1],
                'camera_h_res':m[2],
                'camera_v_res':m[3],
                'q_w':m[4],
                'q_x':m[5],
                'q_y':m[6],
                'q_z':m[7],
                'v_x':m[8],
                'v_y':m[9],
                'v_z':m[10]}
    
    
    nad = np.array([m[8], m[9], m[10]])
    
    #do some edge detecting
    
    image = plt.imread(image_folder+'/'+im)
    
    data = edge_detection.edge_detect(image, 2, 0)
    
    #TODO: 
    #the data coordinates should have origin at the centre of the imaage
    #I think the data from edge_detect has its origin in the top left with y axis pointed down
    #basically how the coordinates of numpy arrays work
    data[:,0] = 60 - data[:,0]
    data[:,1] = data[:,1] - 80
    data = np.flip(data,1)
        
    #centre, r = circle_fit.circle_find(data)
    centre, r = circle_fit.LSfit(data)
    
    vert = ((np.zeros(2) - centre)/np.linalg.norm(np.zeros(2) - centre))*r + centre

    nadir = attitude.find_nadir(r, centre,vert)
    nadir = np.squeeze(np.asarray(nadir))
    
    fig, ax = plt.subplots()
    plt.grid()
    plt.gca().set_aspect("equal")
    plt.xlim(-300, 300)
    plt.ylim(-300, 300)
    plt.scatter(data[:,0],data[:,1])
    plt.scatter(centre[0],centre[1],s=10)
    plt.scatter(vert[0],vert[1],s=100)
    circ = plt.Circle(centre,radius=r, fill=False)
    ax.add_artist(circ)    
    plt.show()
    
    test_results.append({'image':im, 'nadir_actual_x':nad[0], 'nadir_actual_y':nad[1], 'nadir_actual_z':nad[2], 'nadir_predicted_x':nadir[0], 'nadir_predicted_y':nadir[1], 'nadir_predicted_z':nadir[2], 'error':np.linalg.norm(nadir-nad)/np.linalg.norm(nad), 'vert_x': vert[0], 'vert_y': vert[1], 'above_horizon': not (r > np.linalg.norm(np.zeros(2) - centre))})
    
    
test_results = pd.DataFrame(test_results)

