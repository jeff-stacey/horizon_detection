#include "client/renderer.h"
#include "common/util.h"
#include "GL/glew.h"
#include <string>
#include <cassert>
#include <cstring>

#include "fast_obj.h"

#define STBI_ONLY_PNG
#include "stb_image.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

static GLint compile_shader(const char* filename, GLuint shader_type)
{
    std::ifstream shader_file;
    shader_file.open(filename, std::ios::ate);
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

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint log_size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        GLchar* info_log = (GLchar*)malloc(log_size);
        glGetShaderInfoLog(shader, log_size, NULL, info_log);
        cout << "Shader compilation failed with the following error message: "
             << info_log
             << endl;
        free(info_log);
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

Mesh::Mesh(void *mesh_vertices, uint32_t num_vertices, uint16_t vertex_size, ShaderProgramId mesh_shader_program)
{
    shader_program = mesh_shader_program;

    assert(vertex_size == sizeof(Vec3));
    for (uint32_t i = 0; i < num_vertices; i++)
    {
        vertices.push_back(
            Vertex(
                ((Vec3*)mesh_vertices)[i],
                Vec3(1.0f, 0.0f, 0.0f),
                Vec2(0.0f, 0.0f)
            )
        );
    }

    // create and fill vbo
    vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        num_vertices * vertex_size,
        mesh_vertices,
        GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    {
        unsigned int stride = vertex_size;
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
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Mesh::Mesh(const char *filename, ShaderProgramId mesh_shader_program)
{
    shader_program = mesh_shader_program;

    // load mesh
    fastObjMesh *mesh = fast_obj_read(filename);

    // for each group
    for (unsigned int g = 0; g < mesh->group_count; g++)
    {
        unsigned int idx_offset = 0;
        fastObjGroup group = mesh->groups[g];
        // for each face
        for (unsigned int f = 0; f < group.face_count; f++)
        {
            unsigned int num_vertices = group.vertices[f];
            if (num_vertices != 3)
            {
                cerr << "Only faces with 3 vertices are supported. This one has " << num_vertices
                     << endl;
            }
            for (unsigned int v = 0; v < num_vertices; v++)
            {
                fastObjIndex idx = group.indices[idx_offset + v];

                Vec3 position;
                Vec3 normal;
                Vec2 uv;

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

                if (mesh->texcoord_count > 0)
                {
                    uv = Vec2(
                        mesh->texcoords[2 * idx.t],
                        mesh->texcoords[2 * idx.t + 1]);
                }

                vertices.push_back(Vertex(position, normal, uv));
            }
            idx_offset += num_vertices;
        }
    }

    fast_obj_destroy(mesh);

    // create and fill vbo
    vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    vao = 0;
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
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

ShaderProgram::ShaderProgram(
    const char* vshader_filename,
    const char* fshader_filename)
{
    vshader = compile_shader(vshader_filename, GL_VERTEX_SHADER);
    fshader = compile_shader(fshader_filename, GL_FRAGMENT_SHADER);

    program = link_program(vshader, fshader);
}

Vertex::Vertex(Vec3 _position, Vec3 _normal, Vec2 _uv)
:position(_position), normal(_normal), uv(_uv) {}

Renderer::Renderer(unsigned int _width, unsigned int _height)
:width(_width), height(_height)
{
    // load gl bindings
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        cerr << "Error loading gl bindings: "
             << glewGetErrorString(err) << endl;
    }

    // gl setup
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    // enable antialiasing
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // load meshes
    for (int mesh_type = 0; mesh_type < MeshType::NUM_MESH_TYPES; mesh_type++)
    {
        assert(meshes.size() == mesh_type);
        // this is really weird... better way?
        // TODO: also, we should load shaders seperatly so there isn't duplication
        switch (mesh_type)
        {
            case MeshType::SHIP:
            {
                ShaderProgramId shader_prog =
                    load_shader("resources/shaders/triangle.vert", "resources/shaders/triangle.frag");
                meshes.push_back(Mesh("resources/models/ship2.obj", shader_prog));
                break;
            }
            case MeshType::PROJECTILE:
            {
                ShaderProgramId shader_prog =
                    load_shader("resources/shaders/triangle.vert", "resources/shaders/triangle.frag");
                meshes.push_back(Mesh("resources/models/projectile.obj", shader_prog));
                break;
            }
            case MeshType::SKYBOX:
            {
                ShaderProgramId shader_prog =
                    load_shader("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");
                meshes.push_back(Mesh("resources/models/skybox.obj", shader_prog));
                break;
            }
            case MeshType::PLANET:
            {
                ShaderProgramId shader_prog =
                    load_shader("resources/shaders/triangle.vert", "resources/shaders/triangle.frag");
                meshes.push_back(Mesh("resources/models/planet.obj", shader_prog));
            } break;
            case MeshType::CROSSHAIR:
            {
                ShaderProgramId shader_prog =
                    load_shader("resources/shaders/crosshair.vert", "resources/shaders/white.frag");
                Vec3 crosshair_mesh[] = { Vec3(-0.5f, -0.5f, 0.0f), Vec3(0.5f, 0.5f, 0.0f), Vec3(-0.5f, 0.5f, 0.0f), Vec3(0.5f, -0.5f, 0.0f) };
                meshes.push_back(Mesh(crosshair_mesh, ARRAY_LENGTH(crosshair_mesh), sizeof(*crosshair_mesh), shader_prog));
            } break;
            case MeshType::BOX:
            {
                ShaderProgramId shader_prog =
                    load_shader("resources/shaders/line.vert", "resources/shaders/white.frag");
                // This is a wireframe of a box
                Vec3 box_mesh[] = {
                    // one face
                    Vec3(-0.5f,-0.5f,-0.5f), Vec3( 0.5f,-0.5f,-0.5f),
                    Vec3( 0.5f,-0.5f,-0.5f), Vec3( 0.5f, 0.5f,-0.5f),
                    Vec3( 0.5f, 0.5f,-0.5f), Vec3(-0.5f, 0.5f,-0.5f),
                    Vec3(-0.5f, 0.5f,-0.5f), Vec3(-0.5f,-0.5f,-0.5f),
                    // opposite face
                    Vec3(-0.5f,-0.5f, 0.5f), Vec3( 0.5f,-0.5f, 0.5f),
                    Vec3( 0.5f,-0.5f, 0.5f), Vec3( 0.5f, 0.5f, 0.5f),
                    Vec3( 0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f),
                    Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f,-0.5f, 0.5f),
                    // connecting faces
                    Vec3(-0.5f,-0.5f,-0.5f), Vec3(-0.5f,-0.5f, 0.5f),
                    Vec3( 0.5f,-0.5f,-0.5f), Vec3( 0.5f,-0.5f, 0.5f),
                    Vec3( 0.5f, 0.5f,-0.5f), Vec3( 0.5f, 0.5f, 0.5f),
                    Vec3(-0.5f, 0.5f,-0.5f), Vec3(-0.5f, 0.5f, 0.5f),
                };
                meshes.push_back(Mesh(box_mesh, ARRAY_LENGTH(box_mesh), sizeof(*box_mesh), shader_prog));
            } break;
            default:
                assert(false); // Unimplemeneted mesh type
        }
    }

    skybox_texture = load_texture("resources/textures/skymap3.png");
    skybox_mesh = MeshType::SKYBOX;
}

ShaderProgramId Renderer::load_shader(const char* vshader, const char* fshader)
{
    shader_programs.push_back(ShaderProgram(vshader, fshader));
    return shader_programs.size() - 1;
}

GLuint Renderer::load_texture(const char* texture_filename)
{
    // load actual texture
    int w, h, n;
    unsigned char* image_data = stbi_load(
        texture_filename,
        &w, &h, &n, 3);  // force 3 components per pixel

    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,  // mipmap level
        GL_RGB,   // internal format
        w,
        h,
        0,  // must be 0
        GL_RGB,   // format
        GL_UNSIGNED_BYTE,
        image_data);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image_data);

    return texture_id;
}

void Renderer::set_screen_size(unsigned int w, unsigned int h)
{
    width = w;
    height = h;
    glViewport(0, 0, width, height);
}

void Renderer::prep()
{
    perspective = Mat44::perspective(0.1f, 10000.0f, (float)height / width);
}

void Renderer::draw_mesh(MeshId mesh_id, Vec3 position, Vec3 scale, Rotor orientation) const
{
    Mat33 rotation_matrix = orientation.to_matrix();
    
    const Mesh* mesh = &meshes[mesh_id];
    glUseProgram(shader_programs[mesh->shader_program].program);
    
    GLuint rotation_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "rotation");
    glUniformMatrix3fv(
        rotation_uniform_location,
        1,       // 1 matrix
        GL_TRUE, // transpose (row to column major)
        (GLfloat*)&rotation_matrix.data);

    GLuint screen_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "screen");
    glUniform2f(screen_uniform_location, (GLfloat)width, (GLfloat)height);

    GLuint origin_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "origin");
    glUniform3fv(origin_uniform_location, 1, (GLfloat*)&position);

    GLuint camera_pos_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_pos");
    glUniform3fv(camera_pos_uniform_location, 1, (GLfloat*)&camera_pos);

    GLuint camera_orientation_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_orientation");
    Mat33 camera_orientation_inverse = camera_orientation.inverse().to_matrix();
    glUniformMatrix3fv(
        camera_orientation_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&camera_orientation_inverse.data);

    GLuint scale_uniform_location =
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "scale");
    glUniform3fv(scale_uniform_location, 1, (GLfloat*)&scale);

    GLuint perspective_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "perspective");
    glUniformMatrix4fv(
        perspective_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&perspective.data);

    glBindVertexArray(mesh->vao);
    glDrawArrays(
        GL_TRIANGLES,
        0,  // starting idx
        (int)mesh->vertices.size()
    );
}

void Renderer::draw_bounding_box(BoundingBox bounding_box, WorldSector reference_frame) const
{
    const Mesh* mesh = &meshes[MeshType::BOX];
    glUseProgram(shader_programs[mesh->shader_program].program);

    Vec3 position_local(
        0.5f * (bounding_box.x1 + bounding_box.x2),
        0.5f * (bounding_box.y1 + bounding_box.y2),
        0.5f * (bounding_box.z1 + bounding_box.z2));
    Vec3 position = relative_to_sector(camera_sector, reference_frame, position_local);
    
    GLuint origin_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "origin");
    glUniform3fv(origin_uniform_location, 1, (GLfloat*)&position);

    GLuint camera_pos_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_pos");
    glUniform3fv(camera_pos_uniform_location, 1, (GLfloat*)&camera_pos);

    GLuint camera_orientation_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_orientation");
    Mat33 camera_orientation_inverse = camera_orientation.inverse().to_matrix();
    glUniformMatrix3fv(
        camera_orientation_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&camera_orientation_inverse.data);

    Vec3 scale(
        bounding_box.x2 - bounding_box.x1,
        bounding_box.y2 - bounding_box.y1,
        bounding_box.z2 - bounding_box.z1);

    GLuint scale_uniform_location =
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "scale");
    glUniform3fv(scale_uniform_location, 1, (GLfloat*)&scale);

    GLuint perspective_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "perspective");
    glUniformMatrix4fv(
        perspective_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&perspective.data);

    glBindVertexArray(mesh->vao);
    glDrawArrays(
        GL_LINES,
        0,  // starting idx
        (int)mesh->vertices.size()
    );
}

void Renderer::draw_skybox() const
{
    const Mesh* mesh = &meshes[skybox_mesh];
    glUseProgram(shader_programs[mesh->shader_program].program);

    GLuint screen_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "screen");
    glUniform2f(screen_uniform_location, (GLfloat)width, (GLfloat)height);

    GLuint camera_pos_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_pos");
    glUniform3fv(camera_pos_uniform_location, 1, (GLfloat*)&camera_pos);

    GLuint camera_orientation_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_orientation");
    Mat33 camera_orientation_inverse = camera_orientation.inverse().to_matrix();
    glUniformMatrix3fv(
        camera_orientation_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&camera_orientation_inverse.data);

    GLuint perspective_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "perspective");
    glUniformMatrix4fv(
        perspective_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&perspective.data);

    glBindVertexArray(mesh->vao);
    glBindTexture(GL_TEXTURE_2D, skybox_texture);
    glDepthMask(GL_FALSE);

    glDrawArrays(
        GL_TRIANGLES,
        0,  // starting idx
        (int)mesh->vertices.size()
    );

    glDepthMask(GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void Renderer::draw_crosshair(Vec3 position, float scale) const
{
    Vec3 scale_v(scale, scale, 1.0f);
    const Mesh* mesh = &meshes[MeshType::CROSSHAIR];
    glUseProgram(shader_programs[mesh->shader_program].program);

    GLuint screen_uniform_location =
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "screen");
    glUniform2f(screen_uniform_location, (GLfloat)width, (GLfloat)height);

    GLuint origin_uniform_location =
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "origin");
    glUniform3fv(origin_uniform_location, 1, (GLfloat*)&position);

    GLuint camera_pos_uniform_location =
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_pos");
    glUniform3fv(camera_pos_uniform_location, 1, (GLfloat*)&camera_pos);

    GLuint camera_orientation_uniform_location =
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "camera_orientation");
    Mat33 camera_orientation_inverse = camera_orientation.inverse().to_matrix();
    glUniformMatrix3fv(
        camera_orientation_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&camera_orientation_inverse.data);

    GLuint perspective_uniform_location = 
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "perspective");
    glUniformMatrix4fv(
        perspective_uniform_location,
        1,
        GL_TRUE,
        (GLfloat*)&perspective.data);

    GLuint scale_uniform_location =
        glGetUniformLocation(
            shader_programs[mesh->shader_program].program,
            "scale");
    glUniform3fv(scale_uniform_location, 1, (GLfloat*)&scale_v);

    glBindVertexArray(mesh->vao);
    glDrawArrays(
        GL_LINES,
        0,  // starting idx
        (int)mesh->vertices.size()
    );
}

void Renderer::clear() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
