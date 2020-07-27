#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sun Jul 26 16:29:02 2020

@author: byron
"""
import numpy as np
import matplotlib.pyplot as plt

def FOBD_remove(P, dperc, dims):
    
    #second order barrel distortion fix
    
    #P: set of distorted points
    #dperc: distortion percentage
    #image dimensions expressed as (width, height)
        
    #corner point
    pc = np.array([dims[0]/2,dims[1]/2])
    rd = np.linalg.norm(pc)
    
    #based on distortion percentage, the true corner point should be
    pc_p = pc/(dperc+1)
    
    k = (pc[0]-pc_p[0])/(pc[0]*(rd**2))
        
    #fix all points
    P_new = []
    p_temp = [0,0]
    for i in range(len(P)):
        p_temp = P[i]
        
        P_new.append(p_temp + p_temp*(k*np.linalg.norm(p_temp)**2))
        
    return np.array(P_new)

def FOBD_add(P, dperc, dims):
    
    #second order barrel distortion fix
    
    #P: set of distorted points
    #dperc: distortion percentage
    #image dimensions expressed as (width, height)
        
    #corner point
    pc = np.array([dims[0]/2,dims[1]/2])
    rd = np.linalg.norm(pc)
    
    #based on distortion percentage, the true corner point should be
    pc_p = pc/(dperc+1)
    
    k = (pc[0]-pc_p[0])/(pc[0]*(rd**2))
        
    #fix all points
    P_new = []
    p_temp = [0,0]
    for i in range(len(P)):
        p_temp = P[i]
        
        P_new.append(p_temp - p_temp*(k*np.linalg.norm(p_temp)**2))
    
    return np.array(P_new)


#%%
    
#data_new = FOBD(data, 0.1, (160,120))


data = []
M,N = 10,10

for a in range(1,M):
    for b in range(1,N):
        data.append([10*a-50,10*b-50])

data = np.array(data)

#%%
plt.grid()
plt.gca().set_aspect("equal")
plt.xlim(-85, 85)
plt.ylim(-65, 65)
plt.scatter(data[:,0],data[:,1],c='b')
plt.show()

data = FOBD_add(data,.1,(160,120))
data_new = FOBD_remove(data,.1,(160,120))
        
        

plt.grid()
plt.gca().set_aspect("equal")
plt.xlim(-85, 85)
plt.ylim(-65, 65)
plt.scatter(data[:,0],data[:,1],c='b')
plt.scatter(data_new[:,0],data_new[:,1],c='r')
