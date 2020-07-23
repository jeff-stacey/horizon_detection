#include "rendering.h"

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <random>

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

void render_frame(RenderState render_state, SimulationState state, uint32_t width, uint32_t height)
{
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(render_state.screen_shader);

    glBindTexture(GL_TEXTURE_2D, render_state.noise_texture);

    GLint location = glGetUniformLocation(render_state.screen_shader, "nadir");
    glUniform3fv(location, 1, (GLfloat*)&state.nadir);

    location = glGetUniformLocation(render_state.screen_shader, "screen_width");
    glUniform1ui(location, width);

    location = glGetUniformLocation(render_state.screen_shader, "screen_height");
    glUniform1ui(location, height);

    location = glGetUniformLocation(render_state.screen_shader, "alpha");
    glUniform1f(location, cosf(asinf(EARTH_RADIUS / (EARTH_RADIUS + state.altitude))));

    glBindVertexArray(render_state.screen_mesh.vao);
    glDrawArrays(
        GL_TRIANGLES,
        0,  // starting idx
        (int) render_state.screen_mesh.size
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}

void export_image(const char* filename, RenderState render_state, SimulationState state)
{
    render_frame(render_state, state, CAMERA_WIDTH, CAMERA_HEIGHT);

    uint8_t pixels[CAMERA_WIDTH * CAMERA_HEIGHT];
    glReadnPixels(
        0, 0,
        CAMERA_WIDTH, CAMERA_HEIGHT,
        GL_RED, // rendering is monochrome, but the buffer isn't
        GL_UNSIGNED_BYTE,
        sizeof(pixels),
        pixels);

    stbi_flip_vertically_on_write(1);
    stbi_write_png(filename, CAMERA_WIDTH, CAMERA_HEIGHT, 1, pixels, CAMERA_WIDTH);
    stbi_flip_vertically_on_write(0);
}

void export_binary(const char* filename, RenderState render_state, SimulationState state)
{
    render_frame(render_state, state, CAMERA_WIDTH, CAMERA_HEIGHT);

    int num_pixels = CAMERA_WIDTH * CAMERA_HEIGHT;

    uint16_t pixels[num_pixels];
    glReadnPixels(
        0, 0,
        CAMERA_WIDTH, CAMERA_HEIGHT,
        GL_RED,             // we want a grayscale image
        GL_UNSIGNED_SHORT,  // and 16-bit pixel values
        sizeof(pixels),
        pixels);

    // The Lepton 3.5 data format actually only uses 14 bits per pixel
    // with the upper two bits set to zero, so we'll discard the two LSb.
    
    for (int i = 0; i < num_pixels; i++)
    {
        pixels[i] = pixels[i] >> 2;
    }

    // The output from glReadnPixels is flipped so we need to swap all the rows
    for (int i = 0; i < CAMERA_HEIGHT / 2; ++i)
    {
        for (int j = 0; j < CAMERA_WIDTH; ++j)
        {
            std::swap(pixels[i*CAMERA_WIDTH + j], pixels[(CAMERA_HEIGHT - i - 1)*CAMERA_WIDTH + j]);
        }
    }

    FILE* fd = fopen(filename, "wb");
    fwrite(pixels, sizeof(uint16_t), num_pixels, fd);
    fclose(fd);
}

void generate_noise(float seed, float stdev, float* noise, size_t length)
{
    std::default_random_engine random_engine(seed);
    std::normal_distribution<float> normal_dist(0.0f, stdev);
    for (size_t i = 0; i < length; ++i)
    {
        noise[i] = normal_dist(random_engine);
    }
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

    // Enable VSync
    if (SDL_GL_SetSwapInterval(1) < 0)
    {
        cerr << "Error enabling vsync" << endl;
    }

    // load gl bindings
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        cerr << "Error loading gl bindings: "
             << glewGetErrorString(err) << endl;
    }
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    {
        struct ScreenVertex
        {
            Vec3 vertex;
            
            float u;
            float v;
        } screen_mesh[] = {{Vec3(-1.0f, -1.0f, 0.0f), 0.0f, 0.0f},
                           {Vec3(1.0f, -1.0f, 0.0f),  1.0f, 0.0f},
                           {Vec3(-1.0f, 1.0f, 0.0f),  0.0f, 1.0f},
                           {Vec3(1.0f, -1.0f, 0.0f),  1.0f, 0.0f},
                           {Vec3(1.0f, 1.0f, 0.0f),   1.0f, 1.0f},
                           {Vec3(-1.0f, 1.0f, 0.0f),  0.0f, 1.0f}};

        // create and fill vbo
        GLuint vbo = 0;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(screen_mesh),
            screen_mesh,
            GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        {
            unsigned int stride = sizeof(*screen_mesh);
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

            location = 1;
            offset = (void*)sizeof(Vec3); // uv
            glEnableVertexAttribArray(location);
            glVertexAttribPointer(
                location,
                2, // # componsnts
                GL_FLOAT,
                GL_FALSE, // normalized int conversion
                stride,
                offset);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        render_state.screen_mesh.vao = vao;
        render_state.screen_mesh.size = sizeof(screen_mesh) / sizeof(*screen_mesh);
    }

    render_state.noise = new float[CAMERA_H_RES * CAMERA_V_RES];
    generate_noise(0.0f, 0.01f, render_state.noise, CAMERA_H_RES * CAMERA_V_RES);

    // noise texture
    {
        glGenTextures(1, &render_state.noise_texture);
        glBindTexture(GL_TEXTURE_2D, render_state.noise_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, CAMERA_H_RES, CAMERA_V_RES, 0, GL_RED, GL_FLOAT, render_state.noise);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    render_state.screen_shader = link_program(
        compile_shader("screen_shader.vert", GL_VERTEX_SHADER),
        compile_shader("screen_shader.frag", GL_FRAGMENT_SHADER));

    return render_state;
}
