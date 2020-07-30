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
#include <string> // For std::stof
#include <random>

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

    bool mag_reading = false;

    float mag_stdev = DEFAULT_MAG_STDEV;

    unsigned int seed = 1;
    unsigned int count = 1;

    std::default_random_engine mag_random_engine;
    std::normal_distribution<float> mag_dist;
};

float random_float()
{
    return (float)rand() / (float)RAND_MAX;
}

void export_all(std::string filename, RenderState render_state, SimulationState sim_state)
{
    std::string png_filename = filename + std::string(".png");
    std::string bin_filename = filename + std::string(".bin");
    std::string hrz_filename = filename + std::string(".hrz");

    export_image(png_filename.c_str(), render_state, sim_state);
    export_binary(bin_filename.c_str(), render_state, sim_state);
    sim_state.save_state(hrz_filename.c_str());
}

void randomize_state(SimulationState* state, FuzzOptions* fuzz)
{
    if (fuzz->orientation)
    {
        state->camera.w = 2.0f * (random_float() - 0.5f);
        state->camera.x = 2.0f * (random_float() - 0.5f);
        state->camera.y = 2.0f * (random_float() - 0.5f);
        state->camera.z = 2.0f * (random_float() - 0.5f);
        state->camera.normalize();
    }
    if (fuzz->magnetometer_orientation)
    {
        state->magnetometer_reference_frame.w = random_float();
        state->magnetometer_reference_frame.x = random_float();
        state->magnetometer_reference_frame.y = random_float();
        state->magnetometer_reference_frame.z = random_float();
        state->magnetometer_reference_frame.normalize();
    }
    if (fuzz->atmosphere_height)
    {
        float t = random_float();
        state->visible_atmosphere_height = MAX_ATMOSPHERE_HEIGHT * t + MIN_ATMOSPHERE_HEIGHT * (1.0f - t);
    }
    if (fuzz->altitude)
    {
        float t = random_float();
        state->altitude = MAX_ALTITUDE * t + MIN_ALTITUDE * (1.0f - t);
    }
    if (fuzz->latitude)
    {
        float t = random_float();
        state->latitude = -90.0f * t + 90.0f * (1.0f - t);
    }
    if (fuzz->longitude)
    {
        float t = random_float();
        state->longitude = -180.0f * t + 180.0f * (1.0f - t);
    }
    if (fuzz->noise_seed)
    {
        // The scaling factor is arbitrary, possibly unnecessary.
        state->noise_seed = rand();
    }
    if (fuzz->noise_stdev)
    {
        float t = random_float();
        state->noise_stdev = MAX_NOISE_STDEV * t + MIN_NOISE_STDEV * (1.0f - t);
    }
    if (fuzz->mag_reading)
    {
        fuzz->mag_dist.param(decltype(fuzz->mag_dist)::param_type(0.0f, fuzz->mag_stdev));
        state->mag_noise.x = fuzz->mag_dist(fuzz->mag_random_engine);
        state->mag_noise.y = fuzz->mag_dist(fuzz->mag_random_engine);
        state->mag_noise.z = fuzz->mag_dist(fuzz->mag_random_engine);
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
        else if (strcmp("--fuzz_options", args[arg_index]) == 0)
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
                else if (strcmp("mag_reading", args[arg_index]) == 0)
                {
                    options.fuzz.mag_reading = true;
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
        else if (strcmp("--fuzz_seed", args[arg_index]) == 0)
        {
            ++arg_index;
            char* seed_end;
            options.fuzz.seed = strtoul(args[arg_index], &seed_end, 10);
            if (*seed_end)
            {
                // Couldn't read the whole number
                usage();
            }
        }
        else if (strcmp("--fuzz_count", args[arg_index]) == 0)
        {
            ++arg_index;
            char* count_end;
            options.fuzz.count = strtoul(args[arg_index], &count_end, 10);
            if (*count_end)
            {
                // Couldn't read the whole number
                usage();
            }
        }
        else if (strcmp("--mag_stdev", args[arg_index]) == 0)
        {
            ++arg_index;
            size_t stdev_end;
            options.fuzz.mag_stdev = std::stof(args[arg_index], &stdev_end);
            if (args[arg_index][stdev_end])
            {
                // Couldn't read the whole number
                usage();
            }
        }
        else
        {
            usage();
        }

        ++arg_index;
    }

    return options;
}

struct GeomagnetismData
{
    MAGtype_MagneticModel* magnetic_models[1];
    MAGtype_Ellipsoid ellipsoid;
    MAGtype_Geoid geoid;
};

void compute_outputs(SimulationState* state, GeomagnetismData geomag)
{
    // calculate nadir vector
    state->nadir = state->camera.inverse().apply_rotation(Vec3(0.0f, 0.0f, -1.0f));

    // Calclate magnetic field
    MAGtype_CoordSpherical spherical_coord;
    MAGtype_CoordGeodetic geo_coord;
    MAGtype_GeoMagneticElements magnetic_field;

    spherical_coord.lambda = state->longitude;
    spherical_coord.phig = state->latitude;
    spherical_coord.r = EARTH_RADIUS + state->altitude;

    MAG_SphericalToGeodetic(geomag.ellipsoid, spherical_coord, &geo_coord);
    MAG_Geomag(geomag.ellipsoid, spherical_coord, geo_coord, geomag.magnetic_models[0], &magnetic_field);

    state->magnetic_field = Vec3(magnetic_field.Y, magnetic_field.X, -magnetic_field.Z);
    state->magnetic_field = state->camera.inverse().apply_rotation(state->magnetic_field);
    Vec3 magnetometer = (0.001f / MAGNETIC_FIELD_SENSITIVITY) * state->magnetometer_reference_frame.apply_rotation(state->magnetic_field) + state->mag_noise;
    // convert magnetometer to int16_t
    state->magnetometer.x = static_cast<int16_t>(roundf(magnetometer.x));
    state->magnetometer.y = static_cast<int16_t>(roundf(magnetometer.y));
    state->magnetometer.z = static_cast<int16_t>(roundf(magnetometer.z));

    // Convert the 3x3 rotation matrix obtained from the quaternion into a 4x4 affine transformation matrix
    // (with zero translation).
    // All the copying has to be done in reverse order so nothing gets overwritten
    state->magnetometer_reference_frame.to_matrix(state->magnetometer_transformation);
    state->magnetometer_transformation[15] = 1.0f;
    state->magnetometer_transformation[14] = 0.0f;
    state->magnetometer_transformation[13] = 0.0f;
    state->magnetometer_transformation[12] = 0.0f;
    state->magnetometer_transformation[11] = 0.0f;
    state->magnetometer_transformation[10] = state->magnetometer_transformation[8];
    state->magnetometer_transformation[9] = state->magnetometer_transformation[7];
    state->magnetometer_transformation[8] = state->magnetometer_transformation[6];
    state->magnetometer_transformation[7] = 0.0f;
    state->magnetometer_transformation[6] = state->magnetometer_transformation[5];
    state->magnetometer_transformation[5] = state->magnetometer_transformation[4];
    state->magnetometer_transformation[4] = state->magnetometer_transformation[3];
    state->magnetometer_transformation[3] = 0.0f;
    // Rest of the values don't need to be moved
}

void start_gui(RenderState render_state, SimulationState state, FuzzOptions fuzz_options, GeomagnetismData geomag)
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
                typing |= ImGui::InputText("Filename", filename_buf, sizeof(filename_buf));
                if (ImGui::Button("Export"))
                {
                    export_all(filename_buf, render_state, state);
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
                ImGui::InputInt("Noise seed", &state.noise_seed);
                ImGui::SliderFloat("Noise stdev", &state.noise_stdev, MIN_NOISE_STDEV, MAX_NOISE_STDEV, "%.4f", 2);
                if (ImGui::Button("Regenerate Noise"))
                {
                    generate_noise(state.noise_seed, state.noise_stdev, &render_state);
                    state.noise_seed += 1;
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

            if (ImGui::TreeNode("Lens Distortion"))
            {
                ImGui::TextWrapped("First and second order coefficients of the Brown-Conrady distortion model, normalized by screen size (i.e. distance to corner is 1). Positive means barrel distortion. Expected K1 is 0.15, expected K2 is 0");
                ImGui::InputFloat("K1", &state.K1, 0.0f, 0.0f, "%.8f");
                ImGui::InputFloat("K2", &state.K2, 0.0f, 0.0f, "%.8f");

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
                ImGui::Checkbox("Magnetometer reading", &fuzz_options.mag_reading);
                ImGui::InputFloat("Magnetometer stdev", &fuzz_options.mag_stdev);
                if (ImGui::Button("Randomize"))
                {
                    randomize_state(&state, &fuzz_options);
                    generate_noise(state.noise_seed, state.noise_stdev, &render_state);
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Outputs"))
            {
                ImGui::Text("Nadir: (%f, %f, %f)", state.nadir.x, state.nadir.y, state.nadir.z);

                ImGui::Text("Magnetic Field (nGauss): (%.0f, %.0f, %.0f)", state.magnetic_field.x, state.magnetic_field.y, state.magnetic_field.z);
                ImGui::Text("Magnetometer Reading: (%d, %d, %d)", state.magnetometer.x, state.magnetometer.y, state.magnetometer.z);

                ImGui::TreePop();
            }
        }

        // ---------------
        // Compute Outputs
        // ---------------
        compute_outputs(&state, geomag);

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

    // Seed fuzz engine
    {
        srand(options.fuzz.seed);

        // Arbitrary and possibly unnecessary scaling factor
        options.fuzz.mag_random_engine.seed(options.fuzz.seed);
    }

    RenderState render_state = render_init(SCREEN_WIDTH_PIXELS, SCREEN_HEIGHT_PIXELS);


    // Init geomagnetism library
    GeomagnetismData geomag;
    {
        // Initialize variable for parameter to avoid warning
        // And get a pointer to the magnetic model array
        // (This API sucks so much)
        char wmm_coeff_filename[] = "WMM_2020/WMM.COF";
        MAGtype_MagneticModel** magnetic_models_ptr = geomag.magnetic_models;
        if(!MAG_robustReadMagModels(wmm_coeff_filename, &magnetic_models_ptr, 1))
        {
            std::cerr << "Magnetic field coefficients file WMM_2020/WMM.COF not found." << std::endl;
        }
        MAG_SetDefaults(&geomag.ellipsoid, &geomag.geoid);
    }

    // If export filename is given then don't run GUI
    if (options.export_filename)
    {
        std::string base_filename(options.export_filename);
        for (unsigned int i = 0; i < options.fuzz.count; ++i)
        {
            randomize_state(&options.loaded_state, &options.fuzz);
            generate_noise(options.loaded_state.noise_seed, options.loaded_state.noise_stdev, &render_state);
            compute_outputs(&options.loaded_state, geomag);
            export_all(base_filename + std::to_string(i), render_state, options.loaded_state);
        }
    }
    else
    {
        start_gui(render_state, options.loaded_state, options.fuzz, geomag);
    }

    SDL_Quit();

    return 0;
}
