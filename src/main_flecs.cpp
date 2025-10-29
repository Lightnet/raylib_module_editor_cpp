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

struct cube_t {
    Vector3 position;
    Vector3 size;
    Color color;
};

struct velocity_t {
    Vector3 value{0,0,0};
};

struct main_context_t {
    Camera3D camera;
};

struct player_controller_t {
    flecs::entity id;
};

struct imgui_test_t {
    bool is_demo;
    bool is_open;
    float f;
    ImVec4 clear_color;
};

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
// end camera mode 3d
void end_camera_mode_3d_system(flecs::iter& it) {
    // TraceLog(LOG_INFO,"End Camera 3D");
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

    flecs::world world = it.world();

    if (!world.has<imgui_test_t>()) { //check for single access
        return; // Singleton destroyed, skip to prevent crashed.
    }
    imgui_test_t& ctx = world.get_mut<imgui_test_t>();


    if (ImGui::Begin("Test Window", &ctx.is_demo))
    {
        ImGui::TextUnformatted(ICON_FA_JEDI);
        ImGui::Text("Test Text.");               // Display some text (you can use a format strings too)
        ImGui::SliderFloat("float", &ctx.f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", &ctx.clear_color.x); // Edit 3 floats representing a color
        if (ImGui::Button("Button")){                            // Buttons return true when clicked (most widgets return true when edited/activated)
            TraceLog(LOG_INFO, "Click");
        }
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
//-----------------------------------------------
// player
//-----------------------------------------------
void render_3d_cube_system(flecs::iter& it) {
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
}
void player_input_system(flecs::iter& it)
{
    flecs::world world = it.world();

    // Guard – player singleton must exist
    if (!world.has<player_controller_t>()) return;

    // Get camera (read-only)
    const main_context_t& ctx = world.get<main_context_t>();
    const Camera3D& cam = ctx.camera;

    // Get the player entity
    const player_controller_t& pc = world.get<player_controller_t>();
    flecs::entity player = pc.id;

    // -----------------------------------------------------------------
    // 1. Build a forward/right vector that lies on the XZ plane
    // -----------------------------------------------------------------
    Vector3 forward = Vector3Subtract(cam.target, cam.position);
    forward.y = 0;                     // keep only XZ
    forward = Vector3Normalize(forward);

    Vector3 right = Vector3CrossProduct(forward, cam.up);
    right = Vector3Normalize(right);

    // -----------------------------------------------------------------
    // 2. Accumulate desired direction
    // -----------------------------------------------------------------
    Vector3 dir{0,0,0};
    const float speed = 5.0f;          // units per second

    if (IsKeyDown(KEY_W)) dir = Vector3Add(dir, forward);
    if (IsKeyDown(KEY_S)) dir = Vector3Subtract(dir, forward);
    if (IsKeyDown(KEY_A)) dir = Vector3Subtract(dir, right);
    if (IsKeyDown(KEY_D)) dir = Vector3Add(dir, right);

    if (Vector3LengthSqr(dir) > 0) {
        dir = Vector3Scale(Vector3Normalize(dir), speed);
    }

    // -----------------------------------------------------------------
    // 3. Write velocity (create component if missing)
    // -----------------------------------------------------------------
    if (!player.has<velocity_t>()) {
        player.add<velocity_t>();
    }
    player.set<velocity_t>({ .value = dir });
}
void player_move_system(flecs::iter& it)
{
    float dt = it.delta_time();

    // Query entities that have both a cube (position) and velocity
    it.world().query<cube_t, velocity_t>()
        .each([dt](cube_t& cube, velocity_t& vel)
    {
        // Simple Euler integration
        cube.position = Vector3Add(cube.position,
                      Vector3Scale(vel.value, dt));

        // Optional: damp velocity if you want “inertia”
        // vel.value = Vector3Scale(vel.value, 0.9f);
    });
}

//-----------------------------------------------
//
//-----------------------------------------------
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
    ecs.system("begin_camera_mode_3d_system")
        .kind(RLBeginModeCamera3D)
        .run(begin_camera_mode_3d_system);
    ecs.system("end_camera_mode_3d_system")
        .kind(RLEndMode3D)
        .run(end_camera_mode_3d_system);
    // player
    ecs.system("player_input_system")
        .kind(RLUpdate)
        .run(player_input_system);
    ecs.system("player_move_system")
        .kind(RLUpdate)               // runs right after player_input_system
        .run(player_move_system);
    ecs.system<cube_t>("render_3d_cube_system")
    .kind(RLRender3D)
    .each([](cube_t& cube) {
        // TraceLog(LOG_INFO, "render???");
        // Each is invoked for each entity
        DrawCubeWires(cube.position, cube.size.x, cube.size.y, cube.size.z, cube.color);
    });
    
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
    ecs.component<player_controller_t>().add(flecs::Singleton);
    // Register component
    ecs.component<imgui_test_t>();
    ecs.component<velocity_t>();
    ecs.component<cube_t>();
}
// --------------------------------------------------------
// main
// --------------------------------------------------------
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

    // Create the world
    flecs::world world;
    // set up
    setup_components(world);
    init_systems(world);

    world.set<main_context_t>({
        .camera = camera
    });

    // world.set(main_context_t{
    //     .camera = camera
    // });

    flecs::entity my_entity = world.entity();
    my_entity.set(cube_t{
        .position = {0.0f, 0.0f, 0.0f},
        .size = {1.0f, 1.0f, 1.0f},
        .color = RED
    });
    flecs::entity my_entity2 = world.entity();
    my_entity2.set(cube_t{
        .position = {3.0f, 0.0f, 0.0f},
        .size = {1.0f, 1.0f, 1.0f},
        .color = GREEN
    });
    world.set(player_controller_t{
        .id = my_entity
    });
    world.set(imgui_test_t{
        .is_demo = true,
        .is_open = true,
        .f = 0.0f,
        .clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f)
    });
    rlImGuiSetup(true); 	// sets up ImGui with ether a dark or light default theme
    // -------------------------------------------------------
    // 2. Main game loop
    // -------------------------------------------------------
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