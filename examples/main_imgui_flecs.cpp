// main.cpp
// https://github.com/SanderMertens/flecs/blob/master/examples/cpp/game_mechanics/scene_management/src/main.cpp
// https://github.com/SanderMertens/flecs/blob/master/examples/cpp/systems/custom_phases/src/main.cpp
#define RAYLIB_IMPLEMENTATION   // <-- tells raylib to include its source once
#include <raylib.h>             // <-- raylib header (C API, works fine in C++)
#include "raymath.h"
#include "imgui.h"
#include "rlImGui.h"	        // include the API header
#include "bake_config.h"

bool open = true;
bool test_open = true;
float f = 0.0f;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void init_systems(flecs::world& ecs) {
    TraceLog(LOG_INFO,    "init_systems");

    ecs.system("PrintTime")
    .kind(flecs::OnUpdate)
    .run([](flecs::iter& it) {

        // inside your game loop, between BeginDrawing() and EndDrawing()
        rlImGuiBegin();			// starts the ImGui content mode. Make all ImGui calls after this
        ImGui::ShowDemoWindow(&open);
        if (ImGui::Begin("Test Window", &test_open))
        {
            ImGui::TextUnformatted(ICON_FA_JEDI);
            ImGui::Text("Test Text.");               // Display some text (you can use a format strings too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", &clear_color.x); // Edit 3 floats representing a color
            if (ImGui::Button("Button")){                            // Buttons return true when clicked (most widgets return true when edited/activated)
                TraceLog(LOG_INFO, "Click...\n");
            }
            // rlImGuiImage(&image);
        }
        ImGui::End();
        rlImGuiEnd();			// ends the ImGui content mode. Make all ImGui calls before this

        // printf("Time: %f\n", it.delta_time());
    });
}

int main()
{
    // -------------------------------------------------------
    // 1. Initialise the window
    // -------------------------------------------------------
    const int screenWidth  = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [C++] – Hello World");
    SetTargetFPS(60);               // 60 FPS

    // -----------------------------------------------------------------
    // 1. Change the global log level (default = LOG_INFO)
    // -----------------------------------------------------------------
    SetTraceLogLevel(LOG_ALL);          // show everything
    // SetTraceLogLevel(LOG_WARNING);   // only warnings + errors + fatal
    // SetTraceLogLevel(LOG_ERROR);     // errors only
    // SetTraceLogLevel(LOG_NONE);      // silence everything

    // -----------------------------------------------------------------
    // 2. Print messages
    // -----------------------------------------------------------------
    // TraceLog(LOG_INFO,    "Program started – FPS target: %d", GetTargetFPS());
    // TraceLog(LOG_WARNING, "This is a warning – you might want to check something");
    // TraceLog(LOG_ERROR,   "Something went wrong! Error code: %d", 42);
    // TraceLog(LOG_DEBUG,   "Debug value: x = %.2f, y = %.2f", 1.23f, 4.56f);
    TraceLog(LOG_INFO,    "RAYLIB INIT LOOP...");


    // Create the world
    flecs::world ecs;

    init_systems(ecs);

    rlImGuiSetup(true); 	// sets up ImGui with ether a dark or light default theme

    // -------------------------------------------------------
    // 2. Main game loop
    // -------------------------------------------------------

    // bool open = true;
    // bool test_open = true;
    // float f = 0.0f;
    // ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // ---- Update ------------------------------------------------
        // (nothing to update in this demo)

        // ---- Draw --------------------------------------------------
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first raylib window in C++!", 40, 200, 20, DARKGRAY);

            DrawFPS(10, 10);   // optional FPS counter

            // // inside your game loop, between BeginDrawing() and EndDrawing()
            // rlImGuiBegin();			// starts the ImGui content mode. Make all ImGui calls after this
            // ImGui::ShowDemoWindow(&open);
            // if (ImGui::Begin("Test Window", &test_open))
            // {
            //     ImGui::TextUnformatted(ICON_FA_JEDI);
            //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //     ImGui::ColorEdit3("clear color", &clear_color.x); // Edit 3 floats representing a color
            //     if (ImGui::Button("Button")){                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //         TraceLog(LOG_INFO, "Click...\n");
            //     }
            //     // rlImGuiImage(&image);
            // }
            // ImGui::End();
            // rlImGuiEnd();			// ends the ImGui content mode. Make all ImGui calls before this

            ecs.progress();
        }
        EndDrawing();
    }

    // -------------------------------------------------------
    // 3. Cleanup
    // -------------------------------------------------------

    rlImGuiShutdown();		// cleans up ImGui
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}