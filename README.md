# raylib_module_editor_cpp

# License: MIT

# library:
- raylib 5.5
- raygui 4.0
- flecs 4.1.1
- imgui 1.92.4
- rlimgui

# Information:
  Sample c++ for raylib with imgui and flecs. Note there will be c code as well for simple libraries. Flecs has c and c++.

# Flecs:
  There was different ways to set up depend build config on the query components.

## singleton
```c++
struct main_context_t {
  Camera3D camera;
};
//...
void begin_camera_mode_3d_system(flecs::iter& it) {
  flecs::world world = it.world();
  if (!world.has<main_context_t>()) return;
  const main_context_t& ctx = world.get<main_context_t>();
  Camera3D& cam = const_cast<Camera3D&>(ctx.camera); // non-const ref
  BeginMode3D(cam);
}
//...
// Create the world
flecs::world world;
world.set(main_context_t{
  .camera = camera
});
```
  Note this is sample and rework later. It to access singleton struct.

# Credits:
- flecs community on discord.
- raylib community from github, reddit and other repo.