
#define RAYLIB_IMPLEMENTATION   // <-- tells raylib to include its source once
#include <raylib.h>
#include "raymath.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <iostream>
#include <cstdarg>
#include <thread>

JPH_SUPPRESS_WARNINGS

// using namespace JPH;
using namespace JPH::literals;
using namespace std;

// ---------------------------------------------------------------------
// Jolt callbacks (unchanged)
static void TraceImpl(const char *inFMT, ...)
{
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);
    cout << buffer << endl;
}

#ifdef JPH_ENABLE_ASSERTS
static bool AssertFailedImpl(const char *inExpression, const char *inMessage,
                             const char *inFile, JPH::uint inLine)
{
    cout << inFile << ":" << inLine << ": (" << inExpression << ") "
         << (inMessage != nullptr ? inMessage : "") << endl;
    return true;
}
#endif
// ---------------------------------------------------------------------

namespace Layers
{
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING    = 1;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

namespace BroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::uint            NUM_LAYERS = 2;
};

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl()
    {
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING]    = BroadPhaseLayers::MOVING;
    }

    // ---- required overrides ------------------------------------------------
    virtual JPH::uint GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

    // ** ALWAYS IMPLEMENT THIS ** – it is a pure virtual in the base class
    virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
    {
        if (inLayer == BroadPhaseLayers::NON_MOVING) return "NON_MOVING";
        if (inLayer == BroadPhaseLayers::MOVING)    return "MOVING";

        JPH_ASSERT(false);
        return "INVALID";
    }

private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer inObject1,
                               JPH::ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
        case Layers::NON_MOVING: return inObject2 == Layers::MOVING;   // static ↔ dynamic
        case Layers::MOVING:     return true;                         // dynamic ↔ anything
        default:                 JPH_ASSERT(false); return false;
        }
    }
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(JPH::ObjectLayer   inObjectLayer,
                               JPH::BroadPhaseLayer inBroadPhaseLayer) const override
    {
        switch (inObjectLayer)
        {
        case Layers::NON_MOVING: return inBroadPhaseLayer == BroadPhaseLayers::MOVING;
        case Layers::MOVING:     return true;
        default:                 JPH_ASSERT(false); return false;
        }
    }
};

class MyContactListener : public JPH::ContactListener
{
public:
    virtual JPH::ValidateResult OnContactValidate(const JPH::Body &, const JPH::Body &, JPH::RVec3Arg, const JPH::CollideShapeResult &) override
    {
        // cout << "Contact validate callback" << endl;
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }
    virtual void OnContactAdded(const JPH::Body &, const JPH::Body &, const JPH::ContactManifold &, JPH::ContactSettings &) override {}
    virtual void OnContactPersisted(const JPH::Body &, const JPH::Body &, const JPH::ContactManifold &, JPH::ContactSettings &) override {}
    virtual void OnContactRemoved(const JPH::SubShapeIDPair &) override {}
};

class MyBodyActivationListener : public JPH::BodyActivationListener
{
public:
    virtual void OnBodyActivated(const JPH::BodyID &, JPH::uint64) override   { /*cout << "Activated" << endl;*/ }
    virtual void OnBodyDeactivated(const JPH::BodyID &, JPH::uint64) override { /*cout << "Deactivated" << endl;*/ }
};

// ---------------------------------------------------------------------
// MAIN
int main(int argc, char** argv)
{
    // -----------------------------------------------------------------
    // 1. Jolt initialisation (unchanged)
    JPH::RegisterDefaultAllocator();
    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    JPH::TempAllocatorImpl temp_allocator(10 * 1024 * 1024);
    JPH::JobSystemThreadPool job_system(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers,
                                   thread::hardware_concurrency() - 1);

    const JPH::uint cMaxBodies = 1024;
    const JPH::uint cNumBodyMutexes = 0;
    const JPH::uint cMaxBodyPairs = 1024;
    const JPH::uint cMaxContactConstraints = 1024;

    BPLayerInterfaceImpl broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;

    JPH::PhysicsSystem physics_system;
    physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                        broad_phase_layer_interface, object_vs_broadphase_layer_filter,
                        object_vs_object_layer_filter);

    MyBodyActivationListener body_activation_listener;
    physics_system.SetBodyActivationListener(&body_activation_listener);

    MyContactListener contact_listener;
    physics_system.SetContactListener(&contact_listener);

    JPH::BodyInterface &body_interface = physics_system.GetBodyInterface();

    // -----------------------------------------------------------------
    // 2. Create floor (static)
    JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
    floor_shape_settings.SetEmbedded();
    JPH::ShapeRefC floor_shape = floor_shape_settings.Create().Get();

    JPH::BodyCreationSettings floor_settings(floor_shape,
                                        JPH::RVec3(0.0_r, -1.0_r, 0.0_r),
                                        JPH::Quat::sIdentity(),
                                        JPH::EMotionType::Static,
                                        Layers::NON_MOVING);
    JPH::Body *floor = body_interface.CreateBody(floor_settings);
    body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

    // -----------------------------------------------------------------
    // 3. Create sphere (dynamic)
    JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f),
                                         JPH::RVec3(0.0_r, 10.0_r, 0.0_r),
                                         JPH::Quat::sIdentity(),
                                         JPH::EMotionType::Dynamic,
                                         Layers::MOVING);
    JPH::BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
    body_interface.SetLinearVelocity(sphere_id, JPH::Vec3(0.0f, -5.0f, 0.0f));

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

    
    // -----------------------------------------------------------------
    // 5. Simulation constants
    const float cDeltaTime = 1.0f / 60.0f;
    const int   cCollisionSteps = 1;

    physics_system.OptimizeBroadPhase();

    JPH::RVec3 sphere_start_pos = JPH::RVec3(0.0_r, 10.0_r, 0.0_r);
    JPH::Quat  sphere_start_rot = JPH::Quat::sIdentity();
    bool is_spectator = false;

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


        // ---- Physics step -------------------------------------------
        physics_system.Update(cDeltaTime, cCollisionSteps,
                              &temp_allocator, &job_system);

        // ---- Get current sphere position (Jolt uses double-precision RVec3) ----
        JPH::RVec3 joltPos = body_interface.GetCenterOfMassPosition(sphere_id);
        Vector3 spherePos = {
            (float)joltPos.GetX(),
            (float)joltPos.GetY(),
            (float)joltPos.GetZ()
        };
        // ---- Reset on R --------------------------------------------
        if (IsKeyPressed(KEY_R))
        {
            body_interface.SetPositionAndRotation(sphere_id, sphere_start_pos, sphere_start_rot, JPH::EActivation::Activate);
            body_interface.SetLinearVelocity (sphere_id, JPH::Vec3(0, -5, 0));
            body_interface.SetAngularVelocity(sphere_id, JPH::Vec3::sZero());
        }

        // ---- Rendering ------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

            // Floor (large green box)
            DrawCube({0, -1, 0}, 200, 2, 200, GREEN);
            DrawCubeWires({0, -1, 0}, 200, 2, 200, DARKGREEN);

            // Sphere (red)
            DrawSphere(spherePos, 0.5f, RED);

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
    // 7. Cleanup
    body_interface.RemoveBody(sphere_id);
    body_interface.DestroyBody(sphere_id);

    body_interface.RemoveBody(floor->GetID());
    body_interface.DestroyBody(floor->GetID());

    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    CloseWindow();
    return 0;
}