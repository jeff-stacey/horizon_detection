#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Jun 15 10:07:38 2020

@author: byron

Based of method from:
    https://iopscience.iop.org/article/10.1088/1742-6596/948/1/012069/pdf
"""


import numpy as np
import matplotlib.pyplot as plt
import math
import edge_detection

image = '/home/byron/Documents/horizon_detection/prototypes/test_images/images/test6.png'
imagedata = '/home/byron/Documents/horizon_detection/prototypes/test_images/metadata/test6.hrz'

def to_euler(q):
    return np.array([math.atan2(2*(q[0]*q[1]+q[2]*q[3]), 1-2*(q[1]**2+q[2]**2)),
                     math.asin( 2*(q[0]*q[2]-q[3]*q[1])),
                     math.atan2(2*(q[0]*q[3]+q[1]*q[2]), 1-2*(q[2]**2+q[3]**2))])
    
    

import struct

ALTITUDE = 500
E_RADIUS = 6378.136

with open(imagedata, 'rb') as f:
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

image = plt.imread(image)

data = edge_detection.edge_detect(image, 2, 0)

#TODO: 
#the data coordinates should have origin at the centre of the imaage
#I think the data from edge_detect has its origin in the top left with y axis pointed down
#basically how the coordinates of numpy arrays work
data[:,0] = 60 - data[:,0]
data[:,1] = data[:,1] - 80
data = np.flip(data,1)


# =============================================================================
# image = plt.imread(image)
# image = image[:,:,2]
# data = []
# plt.imshow(image)
# plt.show()
# 
# for row in range(image.shape[0]):
#     for col in range(image.shape[1]):
#         if image[row,col] < 80:
#             data.append([col-80,60-row])
#             
# data = np.array(data)
# =============================================================================
#data = np.sort(data, axis=1)

plt.xlim(-80, 80)
plt.ylim(-60, 60)
plt.scatter(data[:,0],data[:,1])
plt.show()

#%%

# =============================================================================
# data = [[0,105],
#         [0,-105],
#         [75,0],
#         [-75,0],
#         [68.94,68.94],
#         [-68.94,-68.94],
#         [42.43,-42.43],
#         [-42.43,42.43]]
# =============================================================================

S = []

for x in data:
    S.append([x[0], x[1], 1, x[0]**2, math.sqrt(2)*x[0]*x[1], x[1]**2])

Q,R = np.linalg.qr(S,mode='complete')


R = [[R[0:3,0:3],R[0:3,3:6]],
     [R[3:6,0:3],R[3:6,3:6]]]

U,s,V = np.linalg.svd(R[1][1])

w = V[:,2]

v = -1*np.matrix(np.linalg.inv(R[0][0]))*np.matrix(R[0][1])*w[np.newaxis].T
v = np.array(v.T)[0]

a = np.concatenate((w,v))


#%%
def plot_hyperbola(a0,a1,a2,a3,a4,a5):
    delta = 1
    xrange = np.arange(-80, 80, delta)
    yrange = np.arange(-60, 60, delta)
    X, Y = np.meshgrid(xrange,yrange)
    
    plt.contour((a0*(X**2) + a1*X*Y + a2*(Y**2) + a3*X + a4*Y + a5), [0])
    #plt.show()
       
plot_hyperbola(a[0], a[1], a[2], a[3], a[4], a[5])

#%%

plt.subplot(221)
plt.xlim(-80, 80)
plt.ylim(-60, 60)
plt.scatter(data_good[:,0],data_good[:,1])

plt.subplot(222)
plt.xlim(-80, 80)
plt.ylim(-60, 60)
plt.scatter(data_bad[:,0],data_bad[:,1])

plt.subplot(223)
a = a_good
plot_hyperbola(a[0], a[1], a[2], a[3], a[4], a[5])

plt.subplot(224)
a = a_bad
plot_hyperbola(a[0], a[1], a[2], a[3], a[4], a[5])






