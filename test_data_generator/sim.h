#pragma once

#include "constants.h"
#include "math3d.h"
#include <stdint.h>

#pragma pack(push, 1)
struct SimulationState
{
    // Inputs
    Quaternion camera = DEFAULT_ORIENTATION;
    Quaternion magnetometer_reference_frame = DEFAULT_MAGNETOMETER_REFERENCE_FRAME;

    float altitude = DEFAULT_ALTITUDE;
    float latitude = DEFAULT_LATITUDE;
    float longitude = DEFAULT_LONGITUDE;

    float noise_seed = DEFAULT_NOISE_SEED;
    float noise_stdev = DEFAULT_NOISE_STDEV;

    float visible_atmosphere_height = DEFAULT_VISIBLE_ATMOSPHERE_HEIGHT;

    // Outputs
    Vec3 nadir;
    Vec3 magnetic_field; // nGauss, in camera reference frame
    struct
    {
        int16_t x;
        int16_t y;
        int16_t z;
    } magnetometer;   // in rotated reference frame

    // This is generated from the magnetometer_reference_frame quaternion
    // This could be an input, but the test data generator already takes
    // a quaternion as input and I don't want to break the format.
    float magnetometer_transformation[16];

    bool load_state(const char* filename);
    void save_state(const char* filename);
};
#pragma pack(pop)
