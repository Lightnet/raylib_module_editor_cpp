
#define RAYLIB_IMPLEMENTATION   // <-- tells raylib to include its source once
#include <raylib.h>
#include "raymath.h"

// ---------------------------------------------------------------------
// MAIN
int main(int argc, char** argv)
{
    // -----------------------------------------------------------------
    // 4. raylib initialisation
    const int screenWidth = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Jolt Physics + raylib - Bouncing Sphere");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 12.0f, 10.0f, 12.0f };   // start position
    camera.target     = (Vector3){ 0.0f, 0.0f, 0.0f };     // will be updated each frame
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // runtime variables
    float camYaw   = atan2f(-12.0f, -12.0f);   // initial yaw  (looking toward origin)
    float camPitch = 0.0f;                    // start level
    float camSpeed = 10.0f;                   // base speed (units/sec)
    float camSpeedMultiplier = 1.0f;          // changed by mouse wheel

    const float SPEED_MIN = 1.0f;
    const float SPEED_MAX = 200.0f;
    const float SPEED_STEP = 1.5f;            // multiplier per wheel notch

    bool is_spectator = false;

    Vector3 spherePos = {0.0f,0.0f,0.0f};

    // -----------------------------------------------------------------
    // 6. Main loop (raylib + physics)
    while (!WindowShouldClose())
    {

        if(IsKeyPressed(KEY_ONE)){
            is_spectator = !is_spectator;
            if(is_spectator){
                TraceLog(LOG_INFO, "is_spectator true");
            }else{
                TraceLog(LOG_INFO, "is_spectator false");
            }
        }

        // ---- 1. Mouse look (right button) ---------------------------------
        if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
            Vector2 delta = GetMouseDelta();
            camYaw   -= delta.x * 0.003f;               // yaw sensitivity
            camPitch -= delta.y * 0.003f;

            // clamp pitch
            const float limit = PI * 0.49f;
            camPitch = fmaxf(-limit, fminf(limit, camPitch));

            HideCursor();
        }
        else
        {
            ShowCursor();
        }

        // ---- 2. Build view vectors ----------------------------------------
        Vector3 forward = {
            cosf(camPitch) * sinf(camYaw),
            sinf(camPitch),
            cosf(camPitch) * cosf(camYaw)
        };
        Vector3 right = {
            cosf(camYaw),
            0.0f,
            -sinf(camYaw)
        };
        Vector3 up = Vector3CrossProduct(right, forward); // world up

        // ---- 3. Keyboard movement -----------------------------------------
        Vector3 move = { 0 };
        if (IsKeyDown(KEY_W)) move = Vector3Add(move, forward);
        if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, forward);
        if (IsKeyDown(KEY_A)) move = Vector3Add(move, right);
        if (IsKeyDown(KEY_D)) move = Vector3Subtract(move, right);
        if (IsKeyDown(KEY_SPACE))      move.y += 1.0f;
        if (IsKeyDown(KEY_LEFT_SHIFT)) move.y -= 1.0f;

        float deltaTime = GetFrameTime();
        if (Vector3LengthSqr(move) > 0.0f)
        {
            move = Vector3Normalize(move);
            float speed = camSpeed * camSpeedMultiplier;
            camera.position = Vector3Add(camera.position,
                                        Vector3Scale(move, speed * deltaTime));
        }

        // ---- 4. Update target (where the camera is looking) ---------------
        camera.target = Vector3Add(camera.position, forward);

        // ---- 5. Mouse wheel speed change ----------------------------------
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
        {
            camSpeedMultiplier *= powf(SPEED_STEP, wheel);
            camSpeedMultiplier = fmaxf(SPEED_MIN / camSpeed,
                                    fminf(SPEED_MAX / camSpeed, camSpeedMultiplier));
        }

        // ---- Rendering ------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

            // Floor (large green box)
            // DrawCube({0, -1, 0}, 200, 2, 200, GREEN);
            // DrawCubeWires({0, -1, 0}, 200, 2, 200, DARKGREEN);

            // Sphere (red)
            DrawSphere(spherePos, 1.0f, RED);
            DrawSphereWires(spherePos, 1.1f, 4, 4, BLACK);

            // Optional grid
            DrawGrid(20, 5.0f);

        EndMode3D();

        // HUD
        DrawText(TextFormat("Step: %.0f   Pos: %.2f, %.2f, %.2f",
                            GetFrameTime() * 60.0f,
                            spherePos.x, spherePos.y, spherePos.z),
                 10, 10, 20, DARKGRAY);

        DrawFPS(10, 40);

        EndDrawing();
    }

    // -----------------------------------------------------------------
    CloseWindow();
    return 0;
}