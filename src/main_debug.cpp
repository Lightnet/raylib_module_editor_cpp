// main.cpp
#define RAYLIB_IMPLEMENTATION   // <-- tells raylib to include its source once
#include <raylib.h>             // <-- raylib header (C API, works fine in C++)

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
    TraceLog(LOG_INFO, "RAYLIB INIT LOOP...\n");
    
    // -------------------------------------------------------
    // 2. Main game loop
    // -------------------------------------------------------
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // ---- Update ------------------------------------------------
        // (nothing to update in this demo)

        // ---- Draw --------------------------------------------------
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first raylib window in C++!", 
                     40, 200, 20, DARKGRAY);

            DrawFPS(10, 10);   // optional FPS counter
        }
        EndDrawing();
    }

    // -------------------------------------------------------
    // 3. Cleanup
    // -------------------------------------------------------
    CloseWindow();                  // Close window and OpenGL context
    return 0;
}