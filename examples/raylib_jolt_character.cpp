
#define RAYLIB_IMPLEMENTATION   // <-- tells raylib to include its source once
#include <raylib.h>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include "Jolt/Physics/PhysicsSettings.h"
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include "Jolt/Physics/Character/CharacterVirtual.h"
#include <Jolt/Core/TempAllocator.h>
#include "Jolt/Core/JobSystem.h"

#include <iostream>
#include <cstdarg>
#include <thread>
#include <vector>
#include <cassert>

JPH_SUPPRESS_WARNINGS

// static JPH::TempAllocatorImpl sTempAllocator(10 * 1024 * 1024);
// JPH::TempAllocatorImpl sallocator(10 * 1024 * 1024); // Uses system allocator
// static JPH::JobSystemThreadPool sJobSystem(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers);

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
    // JPH::TempAllocatorImpl sTempAllocator(10 * 1024 * 1024);

    TraceLog(LOG_INFO,    "init");
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

    JPH::BodyCreationSettings sphere_settings(new JPH::SphereShape(0.5f),
                                         JPH::RVec3(0.0_r, 10.0_r, 0.0_r),
                                         JPH::Quat::sIdentity(),
                                         JPH::EMotionType::Dynamic,
                                         Layers::MOVING);
    JPH::BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
    body_interface.SetLinearVelocity(sphere_id, JPH::Vec3(0.0f, -5.0f, 0.0f));


    // ---------------------------------------------------------------
    //  Character controller settings
    // ---------------------------------------------------------------
    const float character_height_standing = 1.8f;   // capsule height (excluding hemispheres)
    const float character_radius = 0.3f;
    const JPH::RVec3 character_start_pos(0.0_r, 5.0_r, 0.0_r);

    JPH::CharacterVirtualSettings char_settings;
    char_settings.mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
    char_settings.mShape = new JPH::CapsuleShape(character_height_standing * 0.5f, character_radius);
    char_settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -character_radius); // detect ground
    char_settings.mMass = 80.0f;
    //char_settings.mGravity = physics_system.GetGravity();

    JPH::CharacterVirtual* character = new JPH::CharacterVirtual(
        &char_settings, 
        character_start_pos,
        JPH::Quat::sIdentity(),
        0, // user data (optional)
        &physics_system
    );

    // Add the character to the system (it needs a body ID for internal bookkeeping)
    //character->AddToPhysicsSystem(JPH::EActivation::Activate);


    const int screenWidth = 1200;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Jolt Physics + raylib - Bouncing Sphere");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position = { 15.0f, 12.0f, 15.0f };
    camera.target   = { 0.0f, 0.0f, 0.0f };
    camera.up       = { 0.0f, 1.0f, 0.0f };
    camera.fovy    = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    const float cDeltaTime = 1.0f / 60.0f;
    const int   cCollisionSteps = 1;

    physics_system.OptimizeBroadPhase();

    JPH::RVec3 sphere_start_pos = JPH::RVec3(0.0_r, 10.0_r, 0.0_r);
    JPH::Quat  sphere_start_rot = JPH::Quat::sIdentity();

    // ---------------------------------------------------------------
    //  Input helpers
    // ---------------------------------------------------------------
    const float walk_speed = 5.0f;
    const float run_speed  = 9.0f;
    const float jump_impulse = 8.0f;
    bool is_running = false;
    JPH::Vec3 gravity(0, -9.8f, 0);

    TraceLog(LOG_INFO,    "init loop");
    while (!WindowShouldClose())
    {
        const float deltaTime = GetFrameTime();

        // Get character input
        JPH::Vec3 movement = JPH::Vec3::sZero();
        if (IsKeyDown(KEY_W)) movement += JPH::Vec3(0, 0, 1);
        if (IsKeyDown(KEY_S)) movement += JPH::Vec3(0, 0, -1);
        if (IsKeyDown(KEY_A)) movement += JPH::Vec3(1, 0, 0);
        if (IsKeyDown(KEY_D)) movement += JPH::Vec3(-1, 0, 0);

        // Jump
        if (IsKeyPressed(KEY_SPACE) && character->IsSupported())
        {
            JPH::Vec3 vel = character->GetLinearVelocity();
            vel.SetY(jump_impulse);
            character->SetLinearVelocity(vel);
        }

        // Character update
        JPH::Vec3 current_velocity = character->GetLinearVelocity();
        current_velocity += JPH::Vec3(0, -9.8f, 0) * deltaTime;
        current_velocity += movement * 5.0f * deltaTime; // Simplified input force

        character->SetLinearVelocity(current_velocity);

        JPH::CharacterVirtual::ExtendedUpdateSettings update_settings;

        character->ExtendedUpdate(
            deltaTime,
            character->GetUp() * physics_system.GetGravity().Length(),
            update_settings,
            physics_system.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
            physics_system.GetDefaultLayerFilter(Layers::MOVING),
            {},
            {},
            temp_allocator
        );

        physics_system.Update(cDeltaTime, cCollisionSteps,
                              &temp_allocator, &job_system);

        
        JPH::RVec3 joltPos = body_interface.GetCenterOfMassPosition(sphere_id);
        Vector3 spherePos = {
            (float)joltPos.GetX(),
            (float)joltPos.GetY(),
            (float)joltPos.GetZ()
        };
        
        if (IsKeyPressed(KEY_R))
        {
            body_interface.SetPositionAndRotation(sphere_id, sphere_start_pos, sphere_start_rot, JPH::EActivation::Activate);
            body_interface.SetLinearVelocity (sphere_id, JPH::Vec3(0, -5, 0));
            body_interface.SetAngularVelocity(sphere_id, JPH::Vec3::sZero());
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

            // Floor (large green box)
            DrawCube({0, -1, 0}, 200, 2, 200, GREEN);
            DrawCubeWires({0, -1, 0}, 200, 2, 200, DARKGREEN);

            // Sphere (red)
            DrawSphere(spherePos, 0.5f, RED);

            // sync 
            JPH::RVec3 char_pos = character->GetCenterOfMassPosition();
            Vector3 pos = { (float)char_pos.GetX(), (float)char_pos.GetY(), (float)char_pos.GetZ() };
            DrawSphereWires(pos, 1.0f, 8, 8, BLACK);

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