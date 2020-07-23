#pragma once

#include "math3d.h"
#include "sim.h"

#include <SDL/SDL.h>
#include <GL/glew.h>

const size_t CAMERA_H_RES = 160;
const size_t CAMERA_V_RES = 120;

struct Mesh
{
    GLuint vao;
    unsigned int size;
};

struct RenderState
{
    SDL_Window* window = nullptr;
    SDL_GLContext gl_ctxt = nullptr;

    Mesh screen_mesh;
    GLint screen_shader = -1;

    GLuint noise_texture = -1;
    float* noise = nullptr;
};

RenderState render_init(unsigned int screen_width, unsigned int screen_height);
void render_frame(RenderState render_state, SimulationState state, uint32_t width, uint32_t height);
void export_image(const char* filename, RenderState render_state, SimulationState state);
void export_binary(const char* filename, RenderState render_state, SimulationState state);

void generate_noise(float seed, float stdev, float* noise, size_t length);
