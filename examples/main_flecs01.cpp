// main.cpp
// https://github.com/SanderMertens/flecs/blob/master/examples/cpp/game_mechanics/scene_management/src/main.cpp
// https://github.com/SanderMertens/flecs/blob/master/examples/cpp/systems/custom_phases/src/main.cpp
#define RAYLIB_IMPLEMENTATION   // <-- tells raylib to include its source once
#include <raylib.h>             // <-- raylib header (C API, works fine in C++)
#include "raymath.h"

#include "imgui.h"
#include "rlImGui.h"	        // include the API header
#include "bake_config.h"

#include <iostream>

struct Position {
    float x, y;
};

struct Walking { };

bool open = true;
bool test_open = true;
float f = 0.0f;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// Dummy system
void Sys(flecs::iter& it) {
    std::cout << "system " << it.system().name() << "\n";
}

void begin_drawing_system(flecs::iter& it) {
    BeginDrawing();
}

void render_2d_background_color_system(flecs::iter& it) {
    ClearBackground(RAYWHITE);
}

void imgui_begin_system(flecs::iter& it) {
    rlImGuiBegin();
}

void imgui_render_system(flecs::iter& it) {
    if (ImGui::Begin("Test Window", &test_open))
    {
        ImGui::TextUnformatted(ICON_FA_JEDI);
        ImGui::Text("Test Text.");               // Display some text (you can use a format strings too)
        // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        // ImGui::ColorEdit3("clear color", &clear_color.x); // Edit 3 floats representing a color
        // if (ImGui::Button("Button")){                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //     TraceLog(LOG_INFO, "Click...\n");
        // }
        // rlImGuiImage(&image);
    }
    ImGui::End();
}

void imgui_end_system(flecs::iter& it) {
    rlImGuiEnd();
}

void end_drawing_system(flecs::iter& it) {
    EndDrawing();
}


void init_systems(flecs::world& ecs) {
    TraceLog(LOG_INFO, "init_systems");

    // UPDATE RL
    flecs::entity RLUpdate = ecs.entity()
        .add(flecs::Phase)
        .depends_on(flecs::OnUpdate);
    // BeginDrawing
    flecs::entity RLBeginDrawing = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLUpdate);

    flecs::entity RLStartRender = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLBeginDrawing);

    // Camera 3D
    flecs::entity RLBeginModeCamera3D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLStartRender);
    flecs::entity RLRender3D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLBeginModeCamera3D);
    flecs::entity RLEndMode3D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLRender3D);
    // imgui
    flecs::entity RLImguiBegin = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLEndMode3D);
    flecs::entity RLImguiRender = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLImguiBegin);
    flecs::entity RLImguiEnd = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLImguiRender);
    // render 2d
    flecs::entity RLRender2D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLImguiEnd);
    // end render
    flecs::entity RLEndDrawing = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLRender2D);

    // Phases
    ecs.system("begin_drawing_system")
        .kind(RLBeginDrawing)
        .run(begin_drawing_system);
    // background
    ecs.system("render_2d_background_color_system")
        .kind(RLStartRender)
        .run(render_2d_background_color_system);

    ecs.system("imgui_begin_system")
        .kind(RLImguiBegin)
        .run(imgui_begin_system);

    ecs.system("imgui_render_system")
        .kind(RLImguiRender)
        .run(imgui_render_system);

    ecs.system("imgui_end_system")
        .kind(RLImguiEnd)
        .run(imgui_end_system);

    ecs.system("end_drawing_system")
        .kind(RLEndDrawing)
        .run(end_drawing_system);

    // ecs.system("render_imgui")
    // .kind(flecs::OnUpdate)
    // .run([](flecs::iter& it) {
    //     // inside your game loop, between BeginDrawing() and EndDrawing()
    //     rlImGuiBegin();			// starts the ImGui content mode. Make all ImGui calls after this
    //     ImGui::ShowDemoWindow(&open);
    //     if (ImGui::Begin("Test Window", &test_open))
    //     {
    //         ImGui::TextUnformatted(ICON_FA_JEDI);
    //         ImGui::Text("Test Text.");               // Display some text (you can use a format strings too)
    //         ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    //         ImGui::ColorEdit3("clear color", &clear_color.x); // Edit 3 floats representing a color
    //         if (ImGui::Button("Button")){                            // Buttons return true when clicked (most widgets return true when edited/activated)
    //             TraceLog(LOG_INFO, "Click...\n");
    //         }
    //         // rlImGuiImage(&image);
    //     }
    //     ImGui::End();
    //     rlImGuiEnd();			// ends the ImGui content mode. Make all ImGui calls before this
    // });
}

int main()
{
    // -------------------------------------------------------
    // 1. Initialise the window
    // -------------------------------------------------------
    const int screenWidth  = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [C++] Hello World");
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
        // BeginDrawing();
        // {
            // ClearBackground(RAYWHITE);

            // DrawText("Congrats! You created your first raylib window in C++!", 40, 200, 20, DARKGRAY);

            // DrawFPS(10, 10);   // optional FPS counter

            // inside your game loop, between BeginDrawing() and EndDrawing()
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
        // }
        // EndDrawing();
    }

    // -------------------------------------------------------
    // 3. Cleanup
    // -------------------------------------------------------

    rlImGuiShutdown();		// cleans up ImGui
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}