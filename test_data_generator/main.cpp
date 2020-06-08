#include "SDL/SDL.h"
#include "GL/glew.h"

#include "math3d.h"
#include "rendering.h"
#include "keyboard.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

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
