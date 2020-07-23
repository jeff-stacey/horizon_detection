#include "SDL/SDL.h"
#include "GL/glew.h"

#include "math3d.h"
#include "rendering.h"
#include "keyboard.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

extern "C" {
#include "WMM_2020/GeomagnetismHeader.h"
}

#include <iostream>
#include <fstream>
#include <cassert>

using std::cout;
using std::cerr;
using std::endl;

const uint32_t SCREEN_WIDTH_PIXELS = 800;
const uint32_t SCREEN_HEIGHT_PIXELS = 600;
const uint32_t CAMERA_WIDTH_PIXELS = 160;
const uint32_t CAMERA_HEIGHT_PIXELS = 120;
const float FOV_HORIZONTAL = 57 * 3.141592f / 180;
const float ALTITUDE_KM = 500;

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
            if (!options.loaded_state.load_state(args[arg_index]))
            {
                exit(1);
            }
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

void start_gui(RenderState render_state, SimulationState state)
{
    SDL_ShowWindow(render_state.window);

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

    MAGtype_MagneticModel* magnetic_models[1];
    MAGtype_Ellipsoid ellipsoid;
    MAGtype_Geoid geoid;
    MAGtype_CoordSpherical spherical_coord;
    MAGtype_CoordGeodetic geo_coord;

    MAGtype_GeoMagneticElements magnetic_field;

    {
        char wmm_coeff_filename[] = "WMM_2020/WMM.COF";
        MAGtype_MagneticModel** magnetic_models_ptr = magnetic_models;
        if(!MAG_robustReadMagModels(wmm_coeff_filename, &magnetic_models_ptr, 1))
        {
            std::cerr << "Magnetic field coefficients file WMM_2020/WMM.COF not found." << std::endl;
        }
    }

    MAG_SetDefaults(&ellipsoid, &geoid);

    char filename_buf[256] = {};

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
            ImGui::Text("Export Test Data");
            // Hack: -4 in buffer size leaving room for file extension
            typing |= ImGui::InputText("Filename", filename_buf, sizeof(filename_buf) - 4);
            if (ImGui::Button("Export"))
            {
                char* file_extension = filename_buf + strlen(filename_buf);

                strncpy(file_extension, ".png", 4);
                file_extension[4] = 0;
                export_image(filename_buf, render_state, state);

                strncpy(file_extension, ".hrz", 4);
                state.save_state(filename_buf);

                strncpy(file_extension, ".bin", 4);
                export_binary(filename_buf, render_state, state);

                file_extension[0] = 0;
                file_extension[1] = 0;
                file_extension[2] = 0;
                file_extension[3] = 0;
            }

            ImGui::Separator();

            ImGui::Text("Noise");

            ImGui::InputFloat("Noise seed", &state.noise_seed);
            ImGui::SliderFloat("Noise stdev", &state.noise_stdev, 0.0f, 0.2f, "%.4f", 2);
            if (ImGui::Button("Regenerate Noise"))
            {
                generate_noise(state.noise_seed, state.noise_stdev, render_state.noise, CAMERA_H_RES * CAMERA_V_RES);
                glTextureSubImage2D(render_state.noise_texture, 0, 0, 0, CAMERA_H_RES, CAMERA_V_RES, GL_RED, GL_FLOAT, render_state.noise);
                state.noise_seed += 1.0f;
            }

            ImGui::Separator();

            ImGui::Text("Coordinates");

            ImGui::InputFloat("Altitude", &state.altitude);
            ImGui::InputFloat("Latitude (deg)", &state.latitude);
            ImGui::InputFloat("Longitude (deg)", &state.longitude);

            spherical_coord.lambda = state.longitude;
            spherical_coord.phig = state.latitude;

            spherical_coord.r = EARTH_RADIUS + state.altitude;

            MAG_SphericalToGeodetic(ellipsoid, spherical_coord, &geo_coord);
            MAG_Geomag(ellipsoid, spherical_coord, geo_coord, magnetic_models[0], &magnetic_field);

            state.magnetic_field = Vec3(magnetic_field.Y, magnetic_field.X, -magnetic_field.Z);
            state.magnetic_field = state.camera.apply_rotation(state.magnetic_field);
            state.magnetometer = (0.001f / MAGNETIC_FIELD_SENSITIVITY) * state.magnetometer_reference_frame.apply_rotation(state.magnetic_field);
        }

        if (!typing)
        {
            const float rotation_increment = 0.01f;
            float roll = 0.0f;
            float pitch = 0.0f;
            float yaw = 0.0f;

            if (keys.held.a) yaw += rotation_increment;
            if (keys.held.d) yaw -= rotation_increment;

            if (keys.held.w) pitch += rotation_increment;
            if (keys.held.s) pitch -= rotation_increment;

            if (keys.held.q) roll += rotation_increment;
            if (keys.held.e) roll -= rotation_increment;

            state.camera = state.camera * Quaternion::roll(roll) * Quaternion::pitch(pitch) * Quaternion::yaw(yaw);

            ImGui::Separator();

            ImGui::Text("Values");

            // calculate nadir vector
            float camera_matrix[9];
            state.camera.inverse().to_matrix(camera_matrix);
            state.nadir = Vec3(-camera_matrix[2], -camera_matrix[5], -camera_matrix[8]);
            ImGui::Text("Nadir: (%f, %f, %f)", state.nadir.x, state.nadir.y, state.nadir.z);

            ImGui::Text("Magnetic Field (nGauss): (%.0f, %.0f, %.0f)", state.magnetic_field.x, state.magnetic_field.y, state.magnetic_field.z);
            ImGui::Text("Magnetometer Reading: (%.0f, %.0f, %.0f)", state.magnetometer.x, state.magnetometer.y, state.magnetometer.z);
        }

        // ---------
        // Rendering
        // ---------
        render_frame(render_state, state, SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(render_state.window);

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

    RenderState render_state = render_init(SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);

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
