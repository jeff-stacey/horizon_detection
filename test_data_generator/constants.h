#pragma once

#include "math3d.h"
#include <cmath>

// All distances are in KM

/**********************
 * Physical Constants *
 **********************/

#define EARTH_RADIUS 6371.0



/************
 * Defaults *
 ************/

//These are initial values for various parameters, when not supplied as command line input

#define DEFAULT_ALTITUDE 500.0
#define DEFAULT_ORIENTATION Quaternion::pitch(M_PI * 0.4)
#define DEFAULT_MAGNETOMETER_REFERENCE_FRAME Quaternion()

// These are in degrees, since WMM model takes degrees
// This is Victoria, BC :)
#define DEFAULT_LATITUDE 48.4284
#define DEFAULT_LONGITUDE 123.3656

#define DEFAULT_NOISE_SEED 1.0
#define DEFAULT_NOISE_STDEV 0.01

#define DEFAULT_MAG_STDEV 10.0

#define DEFAULT_VISIBLE_ATMOSPHERE_HEIGHT 25.0



/********************
 * Hardware Details *
 ********************/

#define CAMERA_WIDTH 160
#define CAMERA_HEIGHT 120

// 57 degrees in radians
#define HORIZONTAL_FOV 57.0 * M_PI / 180.0

#define MAGNETIC_FIELD_SENSITIVITY 0.14

/**********
 * Bounds *
 **********/

// These are upper and lower bounds for user-adjustable parameters

#define MAX_ALTITUDE 600.0
#define MIN_ALTITUDE 400.0

#define MAX_NOISE_STDEV 0.2
#define MIN_NOISE_STDEV 0.0

#define MAX_ATMOSPHERE_HEIGHT 30.0
#define MIN_ATMOSPHERE_HEIGHT 0.0
