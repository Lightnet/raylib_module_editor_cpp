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
#include <rlgl.h>


const float MOUSE_YAW_SENSITIVITY   = 0.003f;   // radians per pixel
const float MOUSE_PITCH_SENSITIVITY = 0.003f;

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

flecs::entity parent_id;
flecs::entity child_id;

// components
struct cube_t {
    Vector3 position;
    Vector3 size;
    Color color;
};
// struct velocity_t {
//     Vector3 value{0,0,0};
// };
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

// ---------------------------------------------------------------
//  Transform3D – hierarchical 3-D transform (position/rot/scale)
// ---------------------------------------------------------------
struct Transform3D {
    Vector3    position{0,0,0};   // local translation
    Quaternion rotation{0,0,0,1}; // local rotation (identity)
    Vector3    scale{1,1,1};      // local scale
    Matrix     localMatrix{};     // cached local matrix
    Matrix     worldMatrix{};     // cached world matrix
    bool       isDirty{true};     // true → needs recalculation
};

// ------------------------------------------------------------
//  Helpers – world is now const&
// ------------------------------------------------------------
static void mark_hierarchy_dirty(flecs::entity e)
{
    if (!e.has<Transform3D>()) return;

    Transform3D& t = e.get_mut<Transform3D>();
    t.isDirty = true;
    e.modified<Transform3D>();

    e.children([&](flecs::entity child){
        mark_hierarchy_dirty(child);
    });
}

static void update_transform(const flecs::world& world, flecs::entity e, Transform3D& t)
{
    // ---- early-out ------------------------------------------------
    flecs::entity parent = e.parent();
    bool parentDirty = false;
    if (parent && parent.has<Transform3D>()) {
        const Transform3D& pt = parent.get<Transform3D>();
        parentDirty = pt.isDirty;
    }
    if (!t.isDirty && !parentDirty) return;

    // ---- local matrix --------------------------------------------
    Matrix translation = MatrixTranslate(t.position.x, t.position.y, t.position.z);
    Matrix rotation    = QuaternionToMatrix(t.rotation);
    Matrix scaling     = MatrixScale(t.scale.x, t.scale.y, t.scale.z);
    t.localMatrix = MatrixMultiply(scaling, MatrixMultiply(rotation, translation));

    // ---- world matrix --------------------------------------------
    if (!parent) {
        t.worldMatrix = t.localMatrix;
    } else {
        const Transform3D& pt = parent.get<Transform3D>();
        t.worldMatrix = MatrixMultiply(t.localMatrix, pt.worldMatrix);
    }

    // ---- mark children dirty --------------------------------------
    e.children([&](flecs::entity child){
        if (child.has<Transform3D>()) {
            Transform3D& ct = child.get_mut<Transform3D>();
            ct.isDirty = true;
            child.modified<Transform3D>();
        }
    });

    t.isDirty = false;
}

static void update_hierarchy(const flecs::world& world, flecs::entity e)
{
    if (!e.has<Transform3D>()) return;

    Transform3D& t = e.get_mut<Transform3D>();
    update_transform(world, e, t);

    e.children([&](flecs::entity child){
        update_hierarchy(world, child);
    });
}

// ------------------------------------------------------------
//  System – capture the world correctly
// ------------------------------------------------------------
static void Transform3DSystem(flecs::iter& it)
{
    // it.world() is an rvalue → store it in a variable first
    const flecs::world& w = it.world();

    auto q = w.query<Transform3D>();
    q.each([&w](flecs::entity e, Transform3D&) {
        update_hierarchy(w, e);
    });
}

void setup_transform3d(flecs::world& ecs)
{
    // Component
    ecs.component<Transform3D>();

    // System – give it a nice name and put it in a phase you prefer.
    // Here we put it in the same RLUpdate phase you already use for
    // player logic, but you can create a dedicated phase if you want.
    ecs.system("Transform3DSystem")
       .kind(RLUpdate)                 // <-- change to your own phase if desired
       .run(Transform3DSystem);
}


// ---------------------------------------------------------------
// 
// ---------------------------------------------------------------

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


// ------------------------------------------------------------
//  Player input – use get_mut + modified
// ------------------------------------------------------------
void player_input_system(flecs::iter& it)
{
    const flecs::world& world = it.world();
    if (!world.has<player_controller_t>()) return;

    auto& pc = world.get_mut<player_controller_t>();
    flecs::entity player = pc.id;
    if (!player.has<Transform3D>() || !world.has<main_context_t>()) return;

    const auto& ctx = world.get<main_context_t>();
    const Camera3D& cam = ctx.camera;

    // === 1. Build camera-relative forward/right (XZ plane) ===
    Vector3 camForward = Vector3Subtract(cam.target, cam.position);
    camForward.y = 0;
    camForward = Vector3Normalize(camForward);
    Vector3 camRight = Vector3CrossProduct(camForward, cam.up);
    camRight = Vector3Normalize(camRight);

    // === 2. Accumulate movement input ===
    Vector3 move_dir{0,0,0};
    const float speed = 5.0f;

    if(IsKeyPressed(KEY_ONE)){
        pc.id = parent_id;
        return;
    }

    if(IsKeyPressed(KEY_TWO)){
        pc.id = child_id;
        return;
    }


    if (IsKeyDown(KEY_W)) move_dir = Vector3Add(move_dir, camForward);
    if (IsKeyDown(KEY_S)) move_dir = Vector3Subtract(move_dir, camForward);
    if (IsKeyDown(KEY_A)) move_dir = Vector3Subtract(move_dir, camRight);
    if (IsKeyDown(KEY_D)) move_dir = Vector3Add(move_dir, camRight);
    if (Vector3LengthSqr(move_dir) > 0.0f) move_dir = Vector3Normalize(move_dir);

    float dt = it.delta_time();
    Vector3 worldDelta = Vector3Scale(move_dir, speed * dt);

    // === 3. Mouse rotation ===
    Vector2 mouseDelta = GetMouseDelta();
    bool doYaw   = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    bool doPitch = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);

    Transform3D& t = player.get_mut<Transform3D>();

    if (doYaw || doPitch) {
        Quaternion yawQuat   = QuaternionIdentity();
        Quaternion pitchQuat = QuaternionIdentity();

        if (doYaw) {
            float yaw = -mouseDelta.x * MOUSE_YAW_SENSITIVITY;
            yawQuat = QuaternionFromAxisAngle({0,1,0}, yaw);
        }
        if (doPitch) {
            float pitch = -mouseDelta.y * MOUSE_PITCH_SENSITIVITY;
            pitchQuat = QuaternionFromAxisAngle({1,0,0}, pitch);
        }

        t.rotation = QuaternionMultiply(t.rotation, QuaternionMultiply(pitchQuat, yawQuat));
        t.isDirty = true;
    }

    // === 4. Apply movement in local space ===
    if (Vector3LengthSqr(worldDelta) > 0.0f) {
        Matrix rotMat = QuaternionToMatrix(t.rotation);
        Vector3 localDelta = Vector3Transform(worldDelta, rotMat);
        t.position = Vector3Add(t.position, localDelta);
        t.isDirty = true;
    }

    player.modified<Transform3D>();
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

    ecs.system<cube_t, const Transform3D>("render_3d_cube_system")
        .kind(RLRender3D)
        .each([](const cube_t& c, const Transform3D& tr) {
            // push matrix, draw, pop
            rlPushMatrix();
            rlMultMatrixf(MatrixToFloat(tr.worldMatrix));
            DrawCubeWires({0,0,0}, c.size.x, c.size.y, c.size.z, c.color);
            rlPopMatrix();
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
    ecs.component<Transform3D>();
    ecs.component<imgui_test_t>();
    ecs.component<cube_t>();
}
// ----------------------------------------------
// main
// ----------------------------------------------
int main()
{
    // -------------------------------------------------------
    // 1. Initialise the window
    // -------------------------------------------------------
    const int screenWidth  = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [C++] Hello World");
    SetTargetFPS(60);                                               // 60 FPS

    // -----------------------------------------------------------------
    // 1. Change the global log level (default = LOG_INFO)
    // -----------------------------------------------------------------
    SetTraceLogLevel(LOG_ALL);                                      // show everything

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = { 0.0f, 10.0f, 10.0f };                       // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };                           // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };                               // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                         // Camera mode type

    // Create the world
    flecs::world world;
    // set up
    setup_components(world);
    init_systems(world);
    setup_transform3d(world);

    world.set<main_context_t>({
        .camera = camera
    });

    // world.set(main_context_t{
    //     .camera = camera
    // });

    // ---------------------------------------------------
    // 1. Add the component to an entity
    // ---------------------------------------------------
    flecs::entity cube = world.entity()
        .set<Transform3D>({
            .position = {0, 0, 0},
            .rotation = QuaternionFromAxisAngle({0,1,0}, 0), // identity
            .scale    = {1,1,1}
        })
        .set<cube_t>({ .size = {1,1,1}, .color = RED });
    parent_id = cube;
    // ---------------------------------------------------
    // 2. Make a child
    // ---------------------------------------------------
    flecs::entity child = world.entity()
        .child_of(cube)                     // hierarchy!
        .set<Transform3D>({
            .position = {2,0,0},
            .scale    = {0.5f,0.5f,0.5f}
        })
        .set<cube_t>({ .size = {1,1,1}, .color = BLUE });;
    child_id = child;
    flecs::entity sub_cube = world.entity("ChildCube")
    .child_of(child)
    .set<Transform3D>({
        .position = {2, 2, 0},
        .scale    = {0.5f, 0.5f, 0.5f}
    })
    .set<cube_t>({ .size = {1,1,1}, .color = BLUE });

    // test
    world.set(player_controller_t{
        // .id = cube
        .id = child
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