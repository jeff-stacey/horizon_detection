##################################################
#                                                #
#  Simple Edge Detection Comparision Test Script #
#                                                #
##################################################
import PIL.Image as pilim
import matplotlib.pyplot as plt
import matplotlib.image as im
import scipy.signal as sig
import numpy as np

####################################
#              Steps               #
####################################

# 1. Ensure PIL/pillow, matplotlib, numpy and scipy are installed
# 2. Put the image to be evaluated in the same directory as this script
# 3. update the "currentImage" variable with the images filename (include the .png)
# 4. Run the Script!

# NOTE: Don't worry if the output is a black image, the image data is all 0 and 1
#       and so the output will look basically uniform. If you want to visualize the
#       edge just modify Z[i,j] in the 'else' case in edge to bin as shown in the function

####################################
#       Important Variables        #
####################################

currentImage = "C:\\Users\\ryanr\\Desktop\\ECE499\\ws\\horizon_detection\\zynq_sw\\testing\\test_images\\test1.png" #Current image to perform edge detection on
plotShow = True #Toggle comparison plot output

####################################
#            Get Image             #
####################################

image = im.imread(currentImage)
print("Current Image:", currentImage)
print("Data Type:", image.dtype, "\n", "Dimensions:", image.shape)

####################################
#           Function(s)            #
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


####################################
#       Sobel edge detection       #
####################################

#Define Sobel convolution kernels 
Gx = np.array([[ -1, 0, 1], [ -2,  0,  2], [-1, 0, 1]])
Gy = np.array([[ 1, 2, 1], [0, 0, 0], [-1, -2, -1]])

# Perform Convolution (uses zero padding method for the edge pixels)
Cx = sig.convolve2d(image, Gx, mode='same', boundary='fill', fillvalue=0)
Cy = sig.convolve2d(image, Gy, mode='same', boundary='fill', fillvalue=0)

#Combine results
C = np.hypot(Cx,Cy)

#Output Edge Map
C_bin = edge2bin(C, 2)
arr2png(C_bin, 'sobel_BEM.png')

####################################
#     Prewitt edge detection       #
####################################

#Define Prewitt convolution kernels 
Px = np.array([[ 1, 0, -1], [ 1,  0,  -1], [1, 0, -1]])
Py = np.array([[ 1, 1, 1], [0, 0, 0], [-1, -1, -1]])

# Perform Convolution (uses zero padding method for the edge pixels)
Dx = sig.convolve2d(image, Px, mode='same', boundary='fill', fillvalue=0)
Dy = sig.convolve2d(image, Py, mode='same', boundary='fill', fillvalue=0)

#Combine results
D = np.hypot(Dx,Dy)

#Output Edge Map
D_bin = edge2bin(D, 2)
arr2png(D_bin, 'prewitt_BEM.png')

####################################
#       Canny edge detection       #
####################################

# Step 1: Apply Gaussian blur (use Gaussian 3x3 kernel)
B_gauss = (1/16) * np.array([[ 1, 2, 1], [ 2,  4,  2], [1, 2, 1]])
B_blur = sig.convolve2d(image, B_gauss, mode='same', boundary='fill', fillvalue=0)
# 3x3 blur kernel source: https://en.wikipedia.org/wiki/Kernel_(image_processing)


kern1 = np.sqrt(2)/4

# Testing kernels
#Cx = np.array([[ -kern1, 0, kern1], [ -1,  0,  1], [ -kern1, 0, kern1]])
#Cy = np.array([[ kern1, 1, kern1], [0, 0, 0], [-kern1, -1, -kern1]])

#Step 2: Obtain the Sobel edge intensity and direction matricies
Bx = sig.convolve2d(B_blur, Gx, mode='same', boundary='fill', fillvalue=0)
By = sig.convolve2d(B_blur, Gy, mode='same', boundary='fill', fillvalue=0)

B = np.hypot(Bx,By)
theta = np.arctan2(By, Bx)

#Step 3: Non-maximum Suppression (edge thinning)
# Source: https://towardsdatascience.com/canny-edge-detection-step-by-step-in-python-computer-vision-b49c3a2d8123
M,N = B.shape
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
        q = B[i, j+1]
        r = B[i, j-1]
      #angle 45
      elif (22.5 <= angle[i,j] < 67.5):
        q = B[i+1, j-1]
        r = B[i-1, j+1]
      #angle 90
      elif (67.5 <= angle[i,j] < 112.5):
        q = B[i+1, j]
        r = B[i-1, j]
      #angle 135
      elif (112.5 <= angle[i,j] < 157.5):
        q = B[i-1, j-1]
        r = B[i+1, j+1]

      if (B[i,j] >= q) and (B[i,j] >= r):
        Z[i,j] = B[i,j]
      else:
        Z[i,j] = 0

    except IndexError as e:
      pass

#Step 4: Double Thresholding
lowRatio = 0.3
highRatio = 0.67

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


#Output Edge Map
Z_bin = edge2bin(Z_thresh, 2)
arr2png(Z_thresh, 'canny_BEM.png')


# Row Sum Creation for Comparison to C Output
for i in range(0, M):
  column_acc = 0
  for j in range(0, N):
    column_acc += Z_thresh[i, j];

  print("\trow sum for ", i, " is ", column_acc);



########################
#     Plot Settings    #
########################

#Setup plot
f, axarr = plt.subplots(1,2)
axarr[0].set_title("Original Image")
axarr[0].imshow(image)

#Sobel
#axarr[1].set_title("Sobel Edge Map")
#axarr[1].imshow(edge2bin(C, 2))

#Prewitt
#axarr[2].set_title("Prewitt Edge Map")
#axarr[2].imshow(edge2bin(D, 2))

#Halfway Canny
axarr[1].set_title("Canny Edge Map")
axarr[1].imshow(edge2bin(Z_thresh, 2))

# Show plot if desired
if plotShow:
  plt.show()
  plt.close()


