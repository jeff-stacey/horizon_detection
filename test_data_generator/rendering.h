#pragma once

#include "math3d.h"
#include "sim.h"

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

RenderState render_init(unsigned int screen_width, unsigned int screen_height);
void render_frame(RenderState render_state, SimulationState state);
void export_image(const char* filename, RenderState render_state, SimulationState state);
