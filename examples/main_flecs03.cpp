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
// phases
flecs::entity RLUpdate;
flecs::entity RLBeginDrawing;
flecs::entity RLStartRender;
flecs::entity RLBeginModeCamera3D;
flecs::entity RLRender3D;
flecs::entity RLEndMode3D;
flecs::entity RLImguiBegin;
flecs::entity RLImguiRender;
flecs::entity RLImguiEnd;
flecs::entity RLRender2D;
flecs::entity RLEndDrawing;
// flecs::entity ;

struct Position {
    float x, y;
};

struct main_context_t {
    Camera3D camera;
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
// BeginDrawing
void begin_drawing_system(flecs::iter& it) {
    BeginDrawing();
}
// ClearBackground
void render_2d_background_color_system(flecs::iter& it) {
    ClearBackground(RAYWHITE);
}
// begin mode camera 3d
void begin_camera_mode_3d_system(flecs::iter& it) {
    // TraceLog(LOG_INFO,"Begin Camera 3D");
    flecs::world world = it.world();
    if (!world.has<main_context_t>()) { //check for single access
        return; // Singleton destroyed, skip
    }
    // TraceLog(LOG_INFO,"Begin Camera 3D and main_context_t");
    const main_context_t& ctx = world.get<main_context_t>();
    Camera3D& cam = const_cast<Camera3D&>(ctx.camera); // non-const ref
    BeginMode3D(cam);
}
// test
void render_3d_system(flecs::iter& it) {
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
}
// end camera mode 3d
void end_camera_mode_3d_system(flecs::iter& it) {
    // TraceLog(LOG_INFO,"End Camera 3D");
    // flecs::world world = it.world();
    // if (!world.has<main_context_t>()) {
    //     return;
    // }
    flecs::world world = it.world();
    if (!world.has<main_context_t>()) { //check for single access
        return; // Singleton destroyed, skip to prevent crashed.
    }
    EndMode3D();
}
// rlImGuiBegin
void imgui_begin_system(flecs::iter& it) {
    rlImGuiBegin();
}
// imgui set up widgets
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
// rlImGuiEnd
void imgui_end_system(flecs::iter& it) {
    rlImGuiEnd();
}
// EndDrawing
void end_drawing_system(flecs::iter& it) {
    EndDrawing();
}
// setup system functions
void init_systems(flecs::world& ecs) {
    TraceLog(LOG_INFO, "init_systems");

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


    // ecs.system<main_context_t>("begin_camera_mode_3d_system")// does not work
    ecs.system("begin_camera_mode_3d_system")
        .kind(RLBeginModeCamera3D)
        .run(begin_camera_mode_3d_system);

    // ecs.system<main_context_t>("render_3d_system")
    ecs.system("render_3d_system")
        .kind(RLRender3D)
        .run(render_3d_system);

    // ecs.system<main_context_t>("end_camera_mode_3d_system")
    ecs.system("end_camera_mode_3d_system")
        .kind(RLEndMode3D)
        .run(end_camera_mode_3d_system);

}
// set up components
void setup_components(flecs::world& ecs) {
    // UPDATE RL
    RLUpdate = ecs.entity()
        .add(flecs::Phase)
        .depends_on(flecs::OnUpdate);
    // BeginDrawing
    RLBeginDrawing = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLUpdate);

    RLStartRender = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLBeginDrawing);
    // Camera 3D
    RLBeginModeCamera3D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLStartRender);
    RLRender3D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLBeginModeCamera3D);
    RLEndMode3D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLRender3D);
    // imgui
    RLImguiBegin = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLEndMode3D);
    RLImguiRender = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLImguiBegin);
    RLImguiEnd = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLImguiRender);
    // render 2d
    RLRender2D = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLImguiEnd);
    // end render
    RLEndDrawing = ecs.entity()
        .add(flecs::Phase)
        .depends_on(RLRender2D);

    // Register singleton component
    ecs.component<main_context_t>().add(flecs::Singleton);
    // ecs.component<main_context_t>();
}
// main
int main()
{
    // -------------------------------------------------------
    // 1. Initialise the window
    // -------------------------------------------------------
    const int screenWidth  = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [C++] Hello World");
    SetTargetFPS(60);                   // 60 FPS

    // -----------------------------------------------------------------
    // 1. Change the global log level (default = LOG_INFO)
    // -----------------------------------------------------------------
    SetTraceLogLevel(LOG_ALL);          // show everything


    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = { 0.0f, 10.0f, 10.0f };  // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

    // Create the world
    flecs::world world;
    // set up
    setup_components(world);
    init_systems(world);

    // world.set<main_context_t>({
    //     .camera = camera
    // });
    world.set(main_context_t{
        .camera = camera
    });
    

    rlImGuiSetup(true); 	// sets up ImGui with ether a dark or light default theme

    // -------------------------------------------------------
    // 2. Main game loop
    // -------------------------------------------------------

    // bool open = true;
    // bool test_open = true;
    // float f = 0.0f;
    // ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    TraceLog(LOG_INFO,"RAYLIB INIT LOOP...");

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        world.progress();
    }

    // -------------------------------------------------------
    // 3. Cleanup
    // -------------------------------------------------------

    rlImGuiShutdown();		        // cleans up ImGui
    CloseWindow();                  // Close window and OpenGL context

    return 0;
}