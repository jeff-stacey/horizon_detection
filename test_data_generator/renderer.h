#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include "GL/glew.h"

#include "common/mesh.h"
#include "common/math.h"

struct Renderer
{
    // an opengl context must already be available before constructing
    Renderer(unsigned int _width, unsigned int _height);
    //MeshId load_mesh(const char* filename, ShaderProgramId shader_prog_id);
    //void load_skybox(const char* filename, const char* texture_filename, ShaderProgramId shader_prog_id);
    ShaderProgramId load_shader(const char* vshader, const char* fshader);
    GLuint load_texture(const char* texture_filename);
    void set_screen_size(unsigned int w, unsigned int h);
    void draw_mesh(MeshId mesh_id, Vec3 position, Vec3 scale, Rotor orientation) const;
    void draw_bounding_box(BoundingBox bounding_box, WorldSector reference_frame) const;
    void draw_skybox() const;
    void draw_crosshair(Vec3 position, float scale) const;
    void clear() const;
    void prep();

    int width;
    int height;

    Vec3 camera_pos;
    WorldSector camera_sector;
    Rotor camera_orientation;

    Mat44 perspective;

    MeshId skybox_mesh;
    GLuint skybox_texture;

    std::vector<Mesh> meshes;
    std::vector<ShaderProgram> shader_programs;
};
