#pragma once

#include "constants.h"
#include "math3d.h"
#include <stdint.h>

#pragma pack(push, 1)
struct SimulationState
{
    Quaternion camera = DEFAULT_ORIENTATION;
    Quaternion magnetometer_magnetometer_reference_frame = DEFAULT_MAGNETOMETER_REFERENCE_FRAME;

    float altitude = DEFAULT_ALTITUDE;
    float latitude = DEFAULT_LATITUDE;
    float longitude = DEFAULT_LONGITUDE;

    float noise_seed = DEFAULT_NOISE_SEED;
    float noise_stdev = DEFAULT_NOISE_STDEV;

    // Outputs
    Vec3 nadir;
    Vec3 magnetometer;

    bool load_state(const char* filename);
    void save_state(char* filename);
};
#pragma pack(pop)
