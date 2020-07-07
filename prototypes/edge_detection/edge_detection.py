#Copyright (c) 2020 Ryan Blais, Hugo Burd, Byron Kontou, and Jeff Stacey

#Permission is hereby granted, free of charge, to any person obtaining a copy of
#this software and associated documentation files (the "Software"), to deal in
#the Software without restriction, including without limitation the rights to
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
#of the Software, and to permit persons to whom the Software is furnished to do
#so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.

import PIL.Image as pilim
import matplotlib.pyplot as plt
import matplotlib.image as im
import scipy.signal as sig
import numpy as np

####################################
#         Helper Functions         #
####################################

# Basic Conversion from Gradient Image to Binary Edge Map
def edge2bin(E, highThresh):
  M,N = E.shape
  Z = np.zeros((M,N), dtype=np.uint8) #initialize our output array
  for i in range(0,M):
    for j in range(0,N):
      if (E[i,j] < highThresh):
        Z[i,j] = 0
      else:
        Z[i,j] = 1
        #Z[i,j] = 255 
        #if you want to visualize the edge choose 255
  return Z

#Save numpy array in image format
def arr2png(A, fname):
  A_im = pilim.fromarray(A)
  A_im.save(fname, 'png')

#Convert edge map to csv file of edge points using threshold
# Returns the array of edge points
def edge2csv(A, thresh, fname):
  M,N = A.shape
  E = np.empty((0,2), int)

  for i in range(0,M):
    for j in range(0,N):
      if (A[i,j] >= thresh):
        E = np.append(E, [[i, j]], axis=0)
      
  np.savetxt(fname, E, fmt='%1d', delimiter=",")
  return E
      
#################################
#         Edge Detection        #
#################################
#
#   Inputs: image --> 120x160 input image
#           mode  --> 0 - Sobel
#                     1 - Prewitt
#                     2 - Canny
#           output -> 0 - Just Edges (CSV)
#                     1 - Whole Image (PNG)
#

def edge_detect(image, mode, output):

  if (mode == 2):
    #Apply Gaussian blur (use Gaussian 5x5 kernel)
    B_gauss = (1/16) * np.array([[ 1, 2, 1], [ 2,  4,  2], [1, 2, 1]])
    image = sig.convolve2d(image, B_gauss, mode='same', boundary='fill', fillvalue=0)
    # 3x3 blur kernel source: https://en.wikipedia.org/wiki/Kernel_(image_processing)

  #Define Kernel based on the mode
  if(mode == 0 or mode ==2):
    #Define Sobel convolution kernels 
    Kx = np.array([[ -1, 0, 1], [ -2,  0,  2], [-1, 0, 1]])
    Ky = np.array([[ 1, 2, 1], [0, 0, 0], [-1, -2, -1]])
  elif (mode == 1):
    #Define Prewitt convolution kernels 
    Kx = np.array([[ 1, 0, -1], [ 1,  0,  -1], [1, 0, -1]])
    Ky = np.array([[ 1, 1, 1], [0, 0, 0], [-1, -1, -1]])
  else:
    print('Invladid mode entered: ', mode)
    print('\nSetting kernels to Sobel')
    Kx = np.array([[ -1, 0, 1], [ -2,  0,  2], [-1, 0, 1]])
    Ky = np.array([[ 1, 2, 1], [0, 0, 0], [-1, -2, -1]])

  # Perform Convolution (uses zero padding method for the edge pixels)
  Cx = sig.convolve2d(image, Kx, mode='same', boundary='fill', fillvalue=0)
  Cy = sig.convolve2d(image, Ky, mode='same', boundary='fill', fillvalue=0)

  #Combine results
  C = np.hypot(Cx,Cy)
  if (mode == 2):
    theta = np.arctan2(Cy, Cx)

  if (mode == 0 ):
    if (output == 0):
      out = edge2csv(C, 1, "edge_sobel.csv")
    elif (output == 1):
      arr2png(C, 'gradimg_sobel.png')
      out = [];
    else:
      print('Invladid output entered: ', output)
      out = [];
    return out
  elif (mode == 1):
    if (output == 0):
      out = edge2csv(C, 1, "edge_prewitt.csv")
    elif (output == 1):
      arr2png(C, 'gradimg_prewitt.png')
      out = [];
    else:
      print('Invladid output entered: ', output)
      out = [];
    return out
  else:
    pass

  # Non-maximum Suppression (edge thinning)
  # Source: https://towardsdatascience.com/canny-edge-detection-step-by-step-in-python-computer-vision-b49c3a2d8123
  M,N = C.shape
  Z = np.zeros((M,N), dtype=np.float32) #initialize our output array
  angle = theta * 180. / np.pi #init the edge direction map
  angle[angle < 0] += 180 

  for i in range(1, M-1):
    for j in range (1, N-1):
      try:
        q = 255
        r = 255
        #angle 0
        if (0 <= angle[i,j] < 22.5) or (157.5 <= angle[i,j] <= 180):
          q = C[i, j+1]
          r = C[i, j-1]
        #angle 45
        elif (22.5 <= angle[i,j] < 67.5):
          q = C[i+1, j-1]
          r = C[i-1, j+1]
        #angle 90
        elif (67.5 <= angle[i,j] < 112.5):
          q = C[i+1, j]
          r = C[i-1, j]
        #angle 135
        elif (112.5 <= angle[i,j] < 157.5):
          q = C[i-1, j-1]
          r = C[i+1, j+1]

        if (C[i,j] >= q) and (C[i,j] >= r):
          Z[i,j] = C[i,j]
        else:
          Z[i,j] = 0

      except IndexError as e:
        pass

  #Step 4: Double Thresholding
  lowRatio = 0.05
  highRatio = 0.09

  highThresh = Z.max() * highRatio
  lowThresh = highThresh * lowRatio

  Z_thresh = np.zeros((M,N), dtype=np.int32)

  weak = np.int32(25)
  strong = np.int32(255)
    
  strong_i, strong_j = np.where(Z >= highThresh)
  zeros_i, zeros_j = np.where(Z < lowThresh)
    
  weak_i, weak_j = np.where((Z <= highThresh) & (Z >= lowThresh))
    
  Z_thresh[strong_i, strong_j] = strong
  Z_thresh[weak_i, weak_j] = weak

  #Step 5: Hysteresis Edge Tracking
  for i in range(1, M-1):
    for j in range(1, N-1):
      if (Z_thresh[i,j] == weak):
        if((Z_thresh[i+1, j-1] == strong) or (Z_thresh[i+1, j] == strong) or (Z_thresh[i+1, j+1] == strong)
          or (Z_thresh[i, j-1] == strong) or (Z_thresh[i, j+1] == strong)
          or (Z_thresh[i-1, j-1] == strong) or (Z_thresh[i-1, j] == strong) or (Z_thresh[i-1, j+1] == strong)):
          Z_thresh[i, j] = strong
        else:
          Z_thresh[i, j] = 0


  if (output == 0):
    out = edge2csv(Z_thresh, 1, "canny.csv")
  elif (output == 1):
    arr2png(Z_bin, 'canny_BEM.png')
    out = [];
  else:
    print('Invladid output entered: ', output) 
    out = [];
  return out


########################
#       Testing        #
########################

# Important Variables
currentImage = 'test_image_5.png' #Current image to perform edge detection on
plotShow = False #Toggle comparison plot output

# Get Image
image = im.imread(currentImage)
print("Current Image:", currentImage)
print("Data Type:", image.dtype, "\n", "Dimensions:", image.shape)

# Sobel Test
test1 = edge_detect(image, 0, 0)
print('Sobel Edge Index Array:\n', test1)

# Prewitt Test
test2 = edge_detect(image, 1, 0)
print('\nPrewitt Edge Index Array:\n', test2)

# Canney Test
test3 = edge_detect(image, 2, 0)
print('\nCanny Edge Index Array:\n', test3)

########################
#     Plot Settings    #
########################

#Setup plot
#f, axarr = plt.subplots(1,4)
#axarr[0].set_title("Original Image")
#axarr[0].imshow(image)

#Sobel
#axarr[1].set_title("Sobel Edge Map")
#axarr[1].imshow(edge2bin(C, 2))

#Prewitt
#axarr[2].set_title("Prewitt Edge Map")
#axarr[2].imshow(edge2bin(D, 2))

#Halfway Canny
#axarr[3].set_title("Canny Edge Map")
#axarr[3].imshow(edge2bin(Z_thresh, 2))

# Show plot if desired
if plotShow:
  plt.show()
  plt.close()


