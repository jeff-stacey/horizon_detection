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

struct FuzzOptions
{
    bool orientation = false;;
    bool magnetometer_orientation = false;

    bool atmosphere_height = false;

    bool altitude = false;
    bool latitude = false;
    bool longitude = false;

    bool noise_seed = false;
    bool noise_stdev = false;

    int seed = 1;
};

float random_float()
{
    return (float)rand() / (float)RAND_MAX;
}

void randomize_state(SimulationState* state, FuzzOptions fuzz)
{
    if (fuzz.orientation)
    {
        state->camera.w = random_float();
        state->camera.x = random_float();
        state->camera.y = random_float();
        state->camera.z = random_float();
        state->camera.normalize();
    }
    if (fuzz.magnetometer_orientation)
    {
        state->magnetometer_reference_frame.w = random_float();
        state->magnetometer_reference_frame.x = random_float();
        state->magnetometer_reference_frame.y = random_float();
        state->magnetometer_reference_frame.z = random_float();
        state->magnetometer_reference_frame.normalize();
    }
    if (fuzz.atmosphere_height)
    {
        float t = random_float();
        state->visible_atmosphere_height = MAX_ATMOSPHERE_HEIGHT * t + MIN_ATMOSPHERE_HEIGHT * (1.0f - t);
    }
    if (fuzz.altitude)
    {
        float t = random_float();
        state->altitude = MAX_ALTITUDE * t + MIN_ALTITUDE * (1.0f - t);
    }
    if (fuzz.latitude)
    {
        float t = random_float();
        state->latitude = -180.0f * t + 180.0f * (1.0f - t);
    }
    if (fuzz.longitude)
    {
        float t = random_float();
        state->longitude = -90.0f * t + 90.0f * (1.0f - t);
    }
    if (fuzz.noise_seed)
    {
        // The scaling factor is arbitrary.
        // It's probably a good idea for the noise seed to vary more than just 0 to 1
        state->noise_seed = random_float() * 1000.0f;
    }
    if (fuzz.noise_stdev)
    {
        float t = random_float();
        state->noise_stdev = MAX_NOISE_STDEV * t + MIN_NOISE_STDEV * (1.0f - t);
    }
}

struct CommandLineOptions
{
    SimulationState loaded_state;
    FuzzOptions fuzz;
    char* export_filename = nullptr;
};

void usage()
{
    cout << "Usage: ./test_image_generator [--load filename] [--export filename] [--fuzz <fuzz options> end]" << endl;
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
        else if (strcmp("--fuzz", args[arg_index]) == 0)
        {
            while (arg_index < argc)
            {
                ++arg_index;

                if (strcmp("orientation", args[arg_index]) == 0)
                {
                    options.fuzz.orientation = true;
                }
                else if (strcmp("magnetometer_orientation", args[arg_index]) == 0)
                {
                    options.fuzz.magnetometer_orientation = true;
                }
                else if (strcmp("atmosphere_height", args[arg_index]) == 0)
                {
                    options.fuzz.atmosphere_height = true;
                }
                else if (strcmp("altitude", args[arg_index]) == 0)
                {
                    options.fuzz.altitude = true;
                }
                else if (strcmp("latitude", args[arg_index]) == 0)
                {
                    options.fuzz.latitude = true;
                }
                else if (strcmp("longitude", args[arg_index]) == 0)
                {
                    options.fuzz.longitude = true;
                }
                else if (strcmp("noise_seed", args[arg_index]) == 0)
                {
                    options.fuzz.noise_seed = true;
                }
                else if (strcmp("noise_stdev", args[arg_index]) == 0)
                {
                    options.fuzz.noise_stdev = true;
                }
                else if (strcmp("end", args[arg_index]) == 0)
                {
                    break;
                }
                else
                {
                    cout << "Unknown fuzz option" << endl;
                    exit(1);
                }
            }

            if (strcmp("end", args[arg_index]) != 0)
            {
                usage();
            }
        }

        ++arg_index;
    }

    return options;
}

void start_gui(RenderState render_state, SimulationState state, FuzzOptions fuzz_options)
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
            if (ImGui::TreeNode("Export Test Data"))
            {
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

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Atmosphere"))
            {
                ImGui::SliderFloat("Visible Height (km)", &state.visible_atmosphere_height, MIN_ATMOSPHERE_HEIGHT, MAX_ATMOSPHERE_HEIGHT);

                ImGui::TreePop();
            };

            if (ImGui::TreeNode("Noise"))
            {
                ImGui::InputFloat("Noise seed", &state.noise_seed);
                ImGui::SliderFloat("Noise stdev", &state.noise_stdev, MIN_NOISE_STDEV, MAX_NOISE_STDEV, "%.4f", 2);
                if (ImGui::Button("Regenerate Noise"))
                {
                    generate_noise(state.noise_seed, state.noise_stdev, &render_state);
                    state.noise_seed += 1.0f;
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Coordinates"))
            {
                ImGui::InputFloat("Altitude", &state.altitude);
                ImGui::InputFloat("Latitude (deg)", &state.latitude);
                ImGui::InputFloat("Longitude (deg)", &state.longitude);

                ImGui::TreePop();
            }
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

            if (ImGui::TreeNode("Randomization"))
            {
                ImGui::Checkbox("Orientation", &fuzz_options.orientation);
                ImGui::Checkbox("Magnetometer Orientation", &fuzz_options.magnetometer_orientation);
                ImGui::Checkbox("Atmosphere Height", &fuzz_options.atmosphere_height);
                ImGui::Checkbox("Altitude", &fuzz_options.altitude);
                ImGui::Checkbox("Latitude", &fuzz_options.latitude);
                ImGui::Checkbox("Longitude", &fuzz_options.longitude);
                ImGui::Checkbox("Noise Seed", &fuzz_options.noise_seed);
                ImGui::Checkbox("Noise Stdev", &fuzz_options.noise_stdev);
                if (ImGui::Button("Randomize"))
                {
                    randomize_state(&state, fuzz_options);
                    generate_noise(state.noise_seed, state.noise_stdev, &render_state);
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Outputs"))
            {
                ImGui::Text("Nadir: (%f, %f, %f)", state.nadir.x, state.nadir.y, state.nadir.z);

                ImGui::Text("Magnetic Field (nGauss): (%.0f, %.0f, %.0f)", state.magnetic_field.x, state.magnetic_field.y, state.magnetic_field.z);
                ImGui::Text("Magnetometer Reading: (%.0f, %.0f, %.0f)", state.magnetometer.x, state.magnetometer.y, state.magnetometer.z);

                ImGui::TreePop();
            }
        }

        // ---------------
        // Compute Outputs
        // ---------------
        {
            // calculate nadir vector
            float camera_matrix[9];
            state.camera.inverse().to_matrix(camera_matrix);
            state.nadir = Vec3(-camera_matrix[2], -camera_matrix[5], -camera_matrix[8]);

            // Calclate magnetic field
            spherical_coord.lambda = state.longitude;
            spherical_coord.phig = state.latitude;

            spherical_coord.r = EARTH_RADIUS + state.altitude;

            MAG_SphericalToGeodetic(ellipsoid, spherical_coord, &geo_coord);
            MAG_Geomag(ellipsoid, spherical_coord, geo_coord, magnetic_models[0], &magnetic_field);

            state.magnetic_field = Vec3(magnetic_field.Y, magnetic_field.X, -magnetic_field.Z);
            state.magnetic_field = state.camera.apply_rotation(state.magnetic_field);
            state.magnetometer = (0.001f / MAGNETIC_FIELD_SENSITIVITY) * state.magnetometer_reference_frame.apply_rotation(state.magnetic_field);
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

    RenderState render_state = render_init(SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);

    if (options.export_filename)
    {
        export_image(options.export_filename, render_state, options.loaded_state);
    }
    else
    {
        start_gui(render_state, options.loaded_state, options.fuzz);
    }

    SDL_Quit();

    return 0;
}
