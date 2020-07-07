#include "rendering.h"

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>
#include <iostream>
#include <fstream>

using std::cout;
using std::cerr;
using std::endl;

#pragma pack(push, 1)
struct Vertex
{
    Vec3 position;
    Vec3 normal;

    Vertex(Vec3 pos, Vec3 norm): position(pos), normal(norm) {};
};
#pragma pack(pop)

static_assert(sizeof(Vertex) == 2 * sizeof(Vec3), "Vertex has to be packed");

static Mesh load_mesh(const char *filename)
{
    // load mesh
    fastObjMesh *mesh = fast_obj_read(filename);

    std::vector<Vertex> vertices;

    // for each group
    for (unsigned int g = 0; g < mesh->group_count; g++)
    {
        unsigned int idx_offset = 0;
        fastObjGroup group = mesh->groups[g];
        // for each face
        for (unsigned int f = 0; f < group.face_count; f++)
        {
            unsigned int num_vertices = mesh->face_vertices[f];
            if (num_vertices != 3)
            {
                cerr << "Only faces with 3 vertices are supported. This one has " << num_vertices
                     << endl;
            }
            for (unsigned int v = 0; v < num_vertices; v++)
            {
                fastObjIndex idx = mesh->indices[idx_offset + v];

                Vec3 position;
                Vec3 normal;
                //Vec2 uv;

                assert(mesh->position_count > 0);

                position = Vec3(
                    mesh->positions[3 * idx.p],
                    mesh->positions[3 * idx.p + 1],
                    mesh->positions[3 * idx.p + 2]);

                if (mesh->normal_count > 0)
                {
                    normal = Vec3(
                        mesh->normals[3 * idx.n],
                        mesh->normals[3 * idx.n + 1],
                        mesh->normals[3 * idx.n + 2]);
                }

                /*
                if (mesh->texcoord_count > 0)
                {
                    uv = Vec2(
                        mesh->texcoords[2 * idx.t],
                        mesh->texcoords[2 * idx.t + 1]);
                }
                */

                vertices.push_back(Vertex(position, normal));
            }
            idx_offset += num_vertices;
        }
    }

    fast_obj_destroy(mesh);

    // create and fill vbo
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    {
        unsigned int stride = sizeof(Vertex);
        unsigned int location = 0;  // pos
        GLvoid* offset = 0;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(
            location,
            3,  // # components
            GL_FLOAT,
            GL_FALSE, // normalized int conversion
            stride,
            offset);
        
        location = 1; // normal
        offset = (GLvoid*)offsetof(Vertex, normal);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(
            location,
            3,  // # components
            GL_FLOAT,
            GL_FALSE,
            stride,
            offset);

        /*
        location = 2; // texcoords
        offset = (GLvoid*)offsetof(Vertex, uv);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(
            location,
            2,  // # components
            GL_FLOAT,
            GL_FALSE,
            stride,
            offset);
        */
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Mesh result;
    result.vao = vao;
    result.size = vertices.size();

    return result;
}

static GLint compile_shader(const char* filename, GLuint shader_type)
{
    std::ifstream shader_file;
    shader_file.open(filename, std::ios::ate | std::ios::binary);
    size_t file_len = shader_file.tellg();
    GLchar* shader_text = (GLchar*) malloc(file_len + 1); // +1 for null terminator
    shader_file.seekg(0, std::ios::beg);
    shader_file.read(shader_text, file_len);

    // add null terminator
    shader_text[file_len] = 0;

    GLuint shader = glCreateShader(shader_type);

    glShaderSource(shader, 1, &shader_text, NULL);
    glCompileShader(shader);

    free(shader_text);

    GLint log_size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    if (log_size > 1)
    {
        GLchar* info_log = (GLchar*)malloc(log_size);
        glGetShaderInfoLog(shader, log_size, NULL, info_log);
        cout << filename  << " info log: "
             << info_log
             << endl;
        free(info_log);
    }

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        cerr << "Shader compilation failed!" << std::endl;
    }

    return shader;
}

static GLuint link_program(GLint vshader, GLint fshader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint log_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        GLchar* info_log = (GLchar*) malloc(log_size);
        glGetProgramInfoLog(program, log_size, NULL, info_log);
        cerr << "Program linking failed with the following error message: "
             << info_log
             << endl;
        free(info_log);
    }
    return program;
}

static void make_perspective_matrix(float* data, float near, float far, float fov, float aspect_ratio)
{
    float csc_fov = 1.0f / sinf(fov * 0.5f);
    float matrix_data[16] = 
    {
        csc_fov, 0.0f, 0.0f, 0.0f,
        0.0f, aspect_ratio * csc_fov, 0.0f, 0.0f,
        0.0f, 0.0f, -(near + far) / (far - near), -2 * near * far / (far - near),
        0.0f, 0.0f, -1.0f, 0.0f
    };

    memcpy(data, matrix_data, sizeof(matrix_data));
}

static void draw_mesh(Mesh mesh, GLint shader_program, Vec3 position, Vec3 scale, Quaternion camera, float* perspective_matrix)
{
    glUseProgram(shader_program);

    float identity_matrix[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    Vec3 zero_vector;

    float camera_orientation_matrix[9] = {};
    camera.inverse().to_matrix(camera_orientation_matrix);
    
    GLuint rotation_uniform_location = 
        glGetUniformLocation(
            shader_program,
            "rotation");
    glUniformMatrix3fv(
        rotation_uniform_location,
        1,       // 1 matrix
        GL_TRUE, // transpose (row to column major)
        (GLfloat*)identity_matrix);

    GLuint origin_uniform_location = 
        glGetUniformLocation(
            shader_program,
            "origin");
    glUniform3fv(origin_uniform_location, 1, (GLfloat*)&position);

    GLuint camera_pos_uniform_location = 
        glGetUniformLocation(
            shader_program,
            "camera_pos");
    glUniform3fv(camera_pos_uniform_location, 1, (GLfloat*)&zero_vector);

    GLuint camera_orientation_uniform_location = 
        glGetUniformLocation(
            shader_program,
            "camera_orientation");
    glUniformMatrix3fv(
        camera_orientation_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)camera_orientation_matrix);

    GLuint scale_uniform_location =
        glGetUniformLocation(
            shader_program,
            "scale");
    glUniform3fv(scale_uniform_location, 1, (GLfloat*)&scale);

    GLuint perspective_uniform_location = 
        glGetUniformLocation(
            shader_program,
            "perspective");
    glUniformMatrix4fv(
        perspective_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)perspective_matrix);

    glBindVertexArray(mesh.vao);
    glDrawArrays(
        GL_TRIANGLES,
        0,  // starting idx
        (int)mesh.size
    );
}

void render_frame(RenderState render_state, SimulationState state)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float perspective_matrix[16];
    make_perspective_matrix(perspective_matrix, 200.0f, 7000.0f, state.fov_h, (float)state.camera_h_res / state.camera_v_res);

    draw_mesh(
        render_state.earth_mesh,
        render_state.earth_shader,
        Vec3(0.0f, 0.0f, -6871.0f),
        6371.0f * Vec3(1.0f, 1.0f, 1.0f),
        state.camera,
        perspective_matrix);
}

void export_image(const char* filename, RenderState render_state, SimulationState state)
{
    glViewport(0, 0, state.camera_h_res, state.camera_v_res);
    render_frame(render_state, state);

    uint8_t pixels[state.camera_h_res * state.camera_v_res];
    glReadnPixels(
        0, 0,
        state.camera_h_res, state.camera_v_res,
        GL_RED, // rendering is monochrome, but the buffer isn't
        GL_UNSIGNED_BYTE,
        sizeof(pixels),
        pixels);

    stbi_flip_vertically_on_write(1);
    stbi_write_png(filename, state.camera_h_res, state.camera_v_res, 1, pixels, state.camera_h_res);
    stbi_flip_vertically_on_write(0);
}

void export_binary(const char* filename, RenderState render_state, SimulationState state)
{
    glViewport(0, 0, state.camera_h_res, state.camera_v_res);
    render_frame(render_state, state);

    int num_pixels = state.camera_h_res * state.camera_v_res;

    uint16_t pixels[num_pixels];
    glReadnPixels(
        0, 0,
        state.camera_h_res, state.camera_v_res,
        GL_RED,             // we want a grayscale image
        GL_UNSIGNED_SHORT,  // and 16-bit pixel values
        sizeof(pixels),
        pixels);

    // the Lepton 3.5 data format actually only uses 14 bits per pixel
    // with the upper two bits set to zero, so we'll discard the two LSb
    
    for (int i = 0; i < num_pixels; i++)
    {
        pixels[i] = pixels[i] >> 2;
    }

    FILE* fd = fopen(filename, "wb");
    fwrite(pixels, sizeof(uint16_t), num_pixels, fd);
    fclose(fd);
}

RenderState render_init(unsigned int screen_width, unsigned int screen_height)
{
    RenderState render_state;

    SDL_Init(SDL_INIT_VIDEO);

    render_state.window = SDL_CreateWindow(
        "ECE 499 Test Image Generator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screen_width, screen_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    render_state.gl_ctxt = SDL_GL_CreateContext(render_state.window);

    // load gl bindings
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        cerr << "Error loading gl bindings: "
             << glewGetErrorString(err) << endl;
    }
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    render_state.earth_mesh = load_mesh("earth.obj");

    render_state.earth_shader = link_program(
        compile_shader("vshader", GL_VERTEX_SHADER),
        compile_shader("fshader", GL_FRAGMENT_SHADER));

    return render_state;
}
