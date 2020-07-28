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

def KOBD_remove(P, k_params):
    
    P_new = []
    p_temp = [0,0]
    
    for i in range(len(P)):
        p_temp = [0,0]
        
        rd = np.linalg.norm(P[i])
        
        for j in range(len(k_params)):
            p_temp += k_params[j]*rd**(2*(j+1))
            
        p_temp = p_temp*P[i]

        P_new.append(P[i] + p_temp)
        
    return np.array(P_new)
            


#%%
    
#data_new = FOBD(data, 0.1, (160,120))

if __name__=='__main__':
    data = []
    M,N = 10,10
    
    for a in range(1,M):
        for b in range(1,N):
            data.append([10*a-50,10*b-50])
    
    data = np.array(data)
    
    data_orig = data
    
    #%%
    plt.grid()
    plt.gca().set_aspect("equal")
    plt.xlim(-85, 85)
    plt.ylim(-65, 65)
    plt.scatter(data[:,0],data[:,1],c='b')
    plt.show()
    
    data = FOBD_add(data,.9,(160,120))
    data_new = KOBD_remove(data,[0,0])
            
            
    
    plt.grid()
    plt.gca().set_aspect("equal")
    plt.xlim(-85, 85)
    plt.ylim(-65, 65)
    plt.scatter(data[:,0],data[:,1],c='r')
    plt.scatter(data_new[:,0],data_new[:,1],c='b')
    
    mse = 0
    for d_orig, d_new in zip(data_orig, data_new):
        mse += np.linalg.norm(d_orig - d_new)
        
    mse = mse/len(data_new)
