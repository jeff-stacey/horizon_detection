#include "SDL/SDL.h"
#include "GL/glew.h"

#include "keyboard.h"

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>

using std::cout;
using std::cerr;
using std::endl;

const uint32_t SCREEN_WIDTH_PIXELS = 800;
const uint32_t SCREEN_HEIGHT_PIXELS = 600;
const uint32_t CAMERA_WIDTH_PIXELS = 160;
const uint32_t CAMERA_HEIGHT_PIXELS = 120;
const float FOV_HORIZONTAL = 57 * 3.141592f / 180;
const float ALTITUDE_KM = 500;

struct Quaternion
{
    float w = 1.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    void normalize()
    {
        float magnitude = sqrtf(w*w + x*x + y*y + z*z);
        w /= magnitude;
        x /= magnitude;
        y /= magnitude;
        z /= magnitude;
    }

    Quaternion operator*(const Quaternion& rhs) const
    {
        Quaternion result;
        result.w = w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z;
        result.x = w*rhs.x + x*rhs.w + y*rhs.z - z*rhs.y;
        result.y = w*rhs.y - x*rhs.z + y*rhs.w + z*rhs.x;
        result.z = w*rhs.z + x*rhs.y - y*rhs.x + z*rhs.w;

        return result;
    }

    Quaternion inverse() const
    {
        Quaternion result;
        result.w = w;
        result.x = -x;
        result.y = -y;
        result.z = -z;

        return result;
    }

    void to_matrix(float* matrix)
    {
        // assuming magnitude 1
        matrix[0] = 1 - 2 * (y*y + z*z);
        matrix[1] = 2 * (x*y - z*w);
        matrix[2] = 2 * (x*z + y*w);
        matrix[3] = 2 * (x*y + z*w);
        matrix[4] = 1 - 2 * (x*x + z*z);
        matrix[5] = 2 * (y*z - x*w);
        matrix[6] = 2 * (x*z - y*w);
        matrix[7] = 2 * (y*z + x*w);
        matrix[8] = 1 - 2 * (x*x + y*y);
    }

    void print()
    {
        cout << w << ", "
             << x << ", "
             << y << ", "
             << z << endl;
    }

    static Quaternion roll(float angle)
    {
        float cosine = cosf(0.5f * angle);
        float sine   = sinf(0.5f * angle);

        Quaternion result;
        result.w = cosine;
        result.z = sine;

        return result;
    }

    static Quaternion pitch(float angle)
    {
        float cosine = cosf(0.5f * angle);
        float sine   = sinf(0.5f * angle);

        Quaternion result;
        result.w = cosine;
        result.x = sine;

        return result;
    }

    static Quaternion yaw(float angle)
    {
        float cosine = cosf(0.5f * angle);
        float sine   = sinf(0.5f * angle);

        Quaternion result;
        result.w = cosine;
        result.y = sine;

        return result;
    }
};

#pragma pack(push, 1)
struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    Vec3(float x_, float y_, float z_): x(x_), y(y_), z(z_) {};
};

Vec3 operator*(float lhs, const Vec3& rhs)
{
    return Vec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

struct Vertex
{
    Vec3 position;
    Vec3 normal;

    Vertex(Vec3 pos, Vec3 norm): position(pos), normal(norm) {};
};
#pragma pop()

struct Mesh
{
    GLuint vao;
    unsigned int size;
};

Mesh load_mesh(const char *filename)
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

GLuint link_program(GLint vshader, GLint fshader)
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

void make_perspective_matrix(float* data, float near, float far, float fov, float aspect_ratio)
{
    float csc_fov = sinf(fov);
    float matrix_data[16] = 
    {
        csc_fov, 0.0f, 0.0f, 0.0f,
        0.0f, aspect_ratio * csc_fov, 0.0f, 0.0f,
        0.0f, 0.0f, -(near + far) / (far - near), -2 * near * far / (far - near),
        0.0f, 0.0f, -1.0f, 0.0f
    };

    memcpy(data, matrix_data, sizeof(matrix_data));
}

void draw_mesh(Mesh mesh, GLint shader_program, Vec3 position, Vec3 scale, Quaternion camera)
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

    float perspective_matrix[16];
    make_perspective_matrix(perspective_matrix, 200.0f, 7000.0f, FOV_HORIZONTAL, (float)CAMERA_WIDTH_PIXELS / CAMERA_HEIGHT_PIXELS);

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

struct SimulationState
{
    float altitude = 500.0f;
    Quaternion camera;
};

struct CommandLineOptions
{
    SimulationState loaded_state;
    char* export_filename = nullptr;
};

void usage()
{
    cout << "Usage: ./test_image_generator [--load filename] [--export filename]" << endl;
    exit(1);
}

CommandLineOptions parse_args(int argc, char** args)
{
    CommandLineOptions options;
    int arg_index = 1;
    while (arg_index < argc)
    {
        if (strcmp("--load", args[arg_index]) == 0)
        {
            ++arg_index;
            if (arg_index >= argc)
            {
                usage();
            }
            std::ifstream loaded_file(args[arg_index], std::ios::binary);
            loaded_file.read((char*)&options.loaded_state, sizeof(options.loaded_state));
        }
        else if (strcmp("--export", args[arg_index]) == 0)
        {
            ++arg_index;
            if (arg_index >= argc)
            {
                usage();
            }
            options.export_filename = args[arg_index];
        }

        ++arg_index;
    }

    return options;
}

struct RenderState
{
    SDL_Window* window = nullptr;
    SDL_GLContext gl_ctxt = nullptr;
    Mesh earth_mesh;
    GLint earth_shader = -1;
};

void render_frame(RenderState render_state, SimulationState state)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_mesh(
        render_state.earth_mesh,
        render_state.earth_shader,
        Vec3(0.0f, 0.0f, -6871.0f),
        6371.0f * Vec3(1.0f, 1.0f, 1.0f),
        state.camera);
}

void export_image(const char* filename, RenderState render_state, SimulationState state)
{
    glViewport(0, 0, CAMERA_WIDTH_PIXELS, CAMERA_HEIGHT_PIXELS);
    render_frame(render_state, state);

    uint8_t pixels[CAMERA_WIDTH_PIXELS * CAMERA_HEIGHT_PIXELS];
    glReadnPixels(
        0, 0,
        CAMERA_WIDTH_PIXELS, CAMERA_HEIGHT_PIXELS,
        GL_RED, // rendering is monochrome, but the buffer isn't
        GL_UNSIGNED_BYTE,
        sizeof(pixels),
        pixels);

    stbi_flip_vertically_on_write(1);
    stbi_write_png(filename, CAMERA_WIDTH_PIXELS, CAMERA_HEIGHT_PIXELS, 1, pixels, CAMERA_WIDTH_PIXELS);
}

RenderState render_init()
{
    RenderState render_state;

    SDL_Init(SDL_INIT_VIDEO);

    render_state.window = SDL_CreateWindow(
        "ECE 499 Test Image Generator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS,
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

void start_gui(RenderState render_state, SimulationState state)
{
    SDL_ShowWindow(render_state.window);

    glViewport(0, 0, SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);

    // Initialize ImGui
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        const char* glsl_version = "#version 330";
        ImGui_ImplSDL2_InitForOpenGL(render_state.window, render_state.gl_ctxt);
        ImGui_ImplOpenGL3_Init(glsl_version);
        ImGui::StyleColorsDark();
    }

    Keyboard keys;

    bool running = true;
    while (running)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(render_state.window);
        ImGui::NewFrame();

        // ----------
        // User Input
        // ----------
        bool typing = false;  // Used to silence keyboard control when using a textbox
        {
            char save_filename_buf[256];
            typing |= ImGui::InputText("Save File", save_filename_buf, sizeof(save_filename_buf));
            if (ImGui::Button("Save State"))
            {
                std::ofstream save_file(save_filename_buf, std::ios::binary | std::ios::trunc);
                save_file.write((const char*)&state, sizeof(state));
            }

            char image_filename_buf[256];
            typing |= ImGui::InputText("Image File", image_filename_buf, sizeof(image_filename_buf));
            if (ImGui::Button("Export Image"))
            {
                export_image(image_filename_buf, render_state, state);
                glViewport(0, 0, SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);
            }
        }

        if (!typing)
        {
            float roll = 0.0f;
            float pitch = 0.0f;
            float yaw = 0.0f;

            if (keys.held.a) yaw += 0.1f;
            if (keys.held.d) yaw -= 0.1f;

            if (keys.held.w) pitch += 0.1f;
            if (keys.held.s) pitch -= 0.1f;

            if (keys.held.q) roll += 0.1f;
            if (keys.held.e) roll -= 0.1f;

            state.camera = state.camera * Quaternion::roll(roll) * Quaternion::pitch(pitch) * Quaternion::yaw(yaw);
        }

        // ---------
        // Rendering
        // ---------
        render_frame(render_state, state);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(render_state.window);

        SDL_Delay(100);

        // ------
        // Events
        // ------
        keys.clear_keydowns();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    keys.handle_keydown(event.key.keysym.sym);
                    break;
                case SDL_KEYUP:
                    keys.handle_keyup(event.key.keysym.sym);
                    break;
            }
        }
    }
}

int main(int argc, char** args)
{
    CommandLineOptions options = parse_args(argc, args);

    SimulationState state = options.loaded_state;

    RenderState render_state = render_init();

    if (options.export_filename)
    {
        export_image(options.export_filename, render_state, state);
    }
    else
    {
        start_gui(render_state, state);
    }

    SDL_Quit();

    return 0;
}
