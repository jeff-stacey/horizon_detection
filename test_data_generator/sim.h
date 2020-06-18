#pragma once

#include "math3d.h"
#include <stdint.h>

#pragma pack(push, 1)
struct SimulationState
{
    float altitude = 500.0f;
    float fov_h = 57.0f * 3.14159265f / 180.0f;  // 57 degrees in radians
    uint32_t camera_h_res = 160;
    uint32_t camera_v_res = 120;
    Quaternion camera;
    Vec3 nadir;

    bool load_state(const char* filename);
    void save_state(char* filename);
};
#pragma pack(pop)
