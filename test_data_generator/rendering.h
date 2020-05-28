#pragma once

#include "math3d.h"

#include <SDL/SDL.h>
#include <GL/glew.h>

struct Mesh
{
    GLuint vao;
    unsigned int size;
};

struct RenderState
{
    SDL_Window* window = nullptr;
    SDL_GLContext gl_ctxt = nullptr;
    Mesh earth_mesh;
    GLint earth_shader = -1;
};

struct SimulationState
{
    float altitude = 500.0f;
    float fov_h = 57.0f * 3.14159265f / 180.0f;  // 57 degrees in radians
    unsigned int camera_h_res = 160;
    unsigned int camera_v_res = 120;
    Quaternion camera;
};

RenderState render_init(unsigned int screen_width, unsigned int screen_height);
void render_frame(RenderState render_state, SimulationState state);
void export_image(const char* filename, RenderState render_state, SimulationState state);
