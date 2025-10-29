cmake_minimum_required(VERSION 3.15)
# project(raylib_module_c LANGUAGES C CXX)
project(raylib_module_cpp)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS  OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_POLICY_VERSION_MINIMUM 3.5) #need for ode
# if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
#     file(DOWNLOAD
#         "https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake"
#         "${CMAKE_BINARY_DIR}/cmake/CPM.cmake"
#     )
# endif()
# include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

include(FetchContent)
# Find OpenGL
find_package(OpenGL REQUIRED)
#================================================
# ODE physics 3d
#================================================
# FetchContent_Declare(
#     ode
#     GIT_REPOSITORY https://bitbucket.org/odedevs/ode.git
#     GIT_TAG        0.16.6
#     # EXCLUDE_FROM_ALL #this disable build?
#     # GIT_SHALLOW TRUE
#     # USES_TERMINAL_DOWNLOAD TRUE
# )
# set(ODE_WITH_DEMOS OFF CACHE BOOL "" FORCE)
# set(ODE_WITH_TESTS OFF CACHE BOOL "" FORCE)
# set(ODE_DOUBLE_PRECISION ON CACHE BOOL "" FORCE) # Explicitly enable double precision
# set(ODE_SHARED ON CACHE BOOL "" FORCE) # Build as shared library (DLL)
# FetchContent_MakeAvailable(ode)
# # Modified (safer)
# target_link_options(ODE PRIVATE
#     -static-libgcc                                  # GNU Compiler Collection
#     -static-libstdc++                               # Uncomment if C++ code is used
#     -static                                         # Avoid full static linking to prevent issues with system libraries
# )
#================================================
# Fetch raylib
#================================================
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(raylib)
#================================================
# raygui
#================================================
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(raygui)
#================================================
# Lua
#================================================
# FetchContent_Declare(
#     lua
#     GIT_REPOSITORY https://github.com/lua/lua.git
#     GIT_TAG v5.4.8
#     GIT_SHALLOW TRUE
# )
# FetchContent_MakeAvailable(lua)
# #================================================
# # Lua doesn't have a CMake build, so we manually compile it
# # List Lua source files (core and libraries, excluding lua.c and luac.c)
# #================================================
# set(LUA_SRC
#     ${lua_SOURCE_DIR}/lapi.c
#     ${lua_SOURCE_DIR}/lauxlib.c
#     ${lua_SOURCE_DIR}/lbaselib.c
#     ${lua_SOURCE_DIR}/lcode.c
#     ${lua_SOURCE_DIR}/lcorolib.c
#     ${lua_SOURCE_DIR}/lctype.c
#     ${lua_SOURCE_DIR}/ldblib.c
#     ${lua_SOURCE_DIR}/ldebug.c
#     ${lua_SOURCE_DIR}/ldo.c
#     ${lua_SOURCE_DIR}/ldump.c
#     ${lua_SOURCE_DIR}/lfunc.c
#     ${lua_SOURCE_DIR}/lgc.c
#     ${lua_SOURCE_DIR}/linit.c
#     ${lua_SOURCE_DIR}/liolib.c
#     ${lua_SOURCE_DIR}/llex.c
#     ${lua_SOURCE_DIR}/lmathlib.c
#     ${lua_SOURCE_DIR}/lmem.c
#     ${lua_SOURCE_DIR}/loadlib.c
#     ${lua_SOURCE_DIR}/lobject.c
#     ${lua_SOURCE_DIR}/lopcodes.c
#     ${lua_SOURCE_DIR}/loslib.c
#     ${lua_SOURCE_DIR}/lparser.c
#     ${lua_SOURCE_DIR}/lstate.c
#     ${lua_SOURCE_DIR}/lstring.c
#     ${lua_SOURCE_DIR}/lstrlib.c
#     ${lua_SOURCE_DIR}/ltable.c
#     ${lua_SOURCE_DIR}/ltablib.c
#     ${lua_SOURCE_DIR}/ltm.c
#     ${lua_SOURCE_DIR}/lundump.c
#     ${lua_SOURCE_DIR}/lvm.c
#     ${lua_SOURCE_DIR}/lzio.c
#     ${lua_SOURCE_DIR}/lutf8lib.c
# )
# # Create a static Lua library
# add_library(lua STATIC ${LUA_SRC})
# target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})
#================================================
# ENET
#================================================
# FetchContent_Declare(
#     enet
#     GIT_REPOSITORY https://github.com/zpl-c/enet.git
#     GIT_TAG v2.6.5
#     GIT_SHALLOW TRUE
# )
# set(ENET_TEST  OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(enet)
#================================================
# FLECS
#================================================
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.1.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(flecs)
#================================================
# CGLM
#================================================
# FetchContent_Declare(
#     cglm
#     GIT_REPOSITORY https://github.com/recp/cglm.git
#     GIT_TAG v0.9.6
#     GIT_PROGRESS   TRUE
# )
# set(CGLM_SHARED OFF CACHE BOOL "" FORCE)
# set(CGLM_STATIC ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(cglm)
#================================================
# libevent
#================================================
# FetchContent_Declare(
#     libevent
#     GIT_REPOSITORY https://github.com/libevent/libevent.git
#     GIT_TAG release-2.1.12-stable
#     GIT_PROGRESS   TRUE
# )
# set(EVENT__DISABLE_OPENSSL ON)
# set(EVENT__DISABLE_BENCHMARK ON)
# set(EVENT__DISABLE_TESTS ON)
# set(EVENT__DISABLE_REGRESS ON)
# set(EVENT__DISABLE_THREAD_SUPPORT ON)
# set(EVENT__DISABLE_SAMPLES ON)
# FetchContent_MakeAvailable(libevent)
#================================================
# STB
#================================================
# FetchContent_Declare(
#     stb
#     GIT_REPOSITORY https://github.com/nothings/stb.git
#     GIT_TAG        master
#     EXCLUDE_FROM_ALL #this disable build?
#     GIT_SHALLOW TRUE
#     USES_TERMINAL_DOWNLOAD TRUE
# )
# FetchContent_MakeAvailable(stb)


FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.4
    # EXCLUDE_FROM_ALL #this disable build?
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(imgui)
add_library(imgui STATIC 
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp)
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR})

FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        main
    # EXCLUDE_FROM_ALL #this disable build?
    GIT_SHALLOW TRUE
    USES_TERMINAL_DOWNLOAD TRUE
)
FetchContent_MakeAvailable(rlimgui)

#================================================
# Application
#================================================
set(EXAMPLE_APP ON) #ON OFF bool
# set(EXAMPLE_APP OFF) #ON OFF bool
if(${EXAMPLE_APP})
    message(STATUS "EXPORT APP")
    set(APP_NAME ril)
    set(SRC_FILES
        ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    )
    add_executable(${APP_NAME}
        # icon.rc
        ${SRC_FILES}
        # src/main.cpp
        # src/main_imgui.cpp
        # src/main_imgui_flecs.cpp
        # src/main_flecs03.cpp
        # src/main_flecs04.cpp
        src/main_flecs.cpp
    )

    # Link application with custom_cimgui
    target_link_libraries(${APP_NAME} PRIVATE 
        imgui
        raylib                                          # raylib
        # custom_cimgui                                 # cimgui
        # lua                                             # lua
        flecs                                           # flecs
        # cglm                                            # cglm
    )

    # Include directories for the application
    target_include_directories(${APP_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include                       # include
        # ${lua_SOURCE_DIR}                                   # lua
        # ${cimgui_SOURCE_DIR}                              # imgui
        # ${cimgui_SOURCE_DIR}/imgui                        # imgui
        # ${raylib_SOURCE_DIR}/src                            # raylib include
        # ${raylib_SOURCE_DIR}/src/external/glfw/include      # glfw
        # ${enet_SOURCE_DIR}/include                        # enet
        # ${stb_SOURCE_DIR}                                   # stb
        ${raygui_SOURCE_DIR}/src                            # raygui
        ${rlimgui_SOURCE_DIR}                               # rlimgui
        ${imgui_SOURCE_DIR}                               # rlimgui
        # ${cglm_SOURCE_DIR}/include                          # clgm
    )

    # Set compile definitions for the application
    # this will enable for cimgui windows and graphics defines
    target_compile_definitions(${APP_NAME} PRIVATE
        # CIMGUI_DEFINE_ENUMS_AND_STRUCTS=1       # cimgui
        # IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1      # imgui
        # CIMGUI_USE_GLFW=1                       # cimgui glfw
        # CIMGUI_USE_OPENGL3=1                    # cimgui opengl 330
        # ENET_IMPLEMENTATION=1                   #enet
    )

    # Windows-specific settings
    if(WIN32)
        # ws2_32 # network
        target_link_libraries(${APP_NAME} PRIVATE ws2_32 gdi32 user32 shell32)
        # target_link_options(${APP_NAME} PRIVATE -static-libgcc)

        target_link_options(${APP_NAME} PRIVATE
            -static-libgcc
            -static-libstdc++  # Uncomment if C++ code is used
            -static           # Avoid full static linking to prevent issues with system libraries
        )
    endif()
endif()

# set(EXPORT_FLECS_APP ON)
set(EXPORT_FLECS_APP OFF)
if(${EXPORT_FLECS_APP})

    set(SRC_FLECS_MODULES
        src/ecs_components.c
        src/module_dev.c
        src/module_enet.c # not there no define...
        src/module_ode.c
        src/module_libevent.c
        src/raygui_impl.c # define RAYGUI_IMPLEMENTATION
        src/enet_impl.c # define ENET_IMPLEMENTATION
    )

    message(STATUS "EXPORT FLECS APP")
    set(APP_FLECS_NAME1 rlm)
    add_executable(${APP_FLECS_NAME1}
        ${SRC_FLECS_MODULES}
        # src/main_camera_capsule.c
        # src/main_raylib_shape_wires.c
        # src/main_camera_picking.c
        # src/main_camera_test01.c
        # src/main_camera_test02.c
        # src/main_editor_test.c
        # src/main_enet_test.c
        # src/main_ode_gui_test.c
        # src/main_ode_test.c
        # src/main_ode_controller_test.c
        # src/main_raylib_libevent_test.c
        src/main_editor_test.c
        # src/main_transform_3d_hierarchy_test.c
        # src/main_transform_3d_hierarchy_test01.c
        # src/main_transform_3d_hierarchy_test02.c
        # src/main.c
        # examples/module/main_transform_3d_hierarchy_test01.c
        # examples/module/main_transform_3d_hierarchy_test02.c
    )

    target_compile_definitions(${APP_FLECS_NAME1} PRIVATE 
        # RAYGUI_IMPLEMENTATION=1
    )

    # Link application with custom_cimgui
    target_link_libraries(${APP_FLECS_NAME1} PRIVATE 
        raylib                                          # raylib
        # lua                                           # lua
        flecs                                           # flecs
        cglm                                            # cglm
        ODE                                             # ode must be cap from ode repo cmake config
        event_shared 
        # event_core_shared 
    )
    # Include directories for the application
    target_include_directories(${APP_FLECS_NAME1} PUBLIC
        ${PROJECT_SOURCE_DIR}/include                       # include
        ${lua_SOURCE_DIR}                                   # lua
        ${raylib_SOURCE_DIR}/src                            # raylib include
        ${raylib_SOURCE_DIR}/src/external/glfw/include      # glfw
        ${enet_SOURCE_DIR}/include                          # enet
        # ${stb_SOURCE_DIR}                                 # stb
        ${raygui_SOURCE_DIR}/src                            # raygui
        ${cglm_SOURCE_DIR}/include                          # clgm
        # ${ode_SOURCE_DIR}/include                         # 
        ${ode_BINARY_DIR}/include                           # 
        ${libevent_SOURCE_DIR}/include                           # 
        ${libevent_BINARY_DIR}/include                           # event2/event-config.h
    )
    # Windows-specific settings
    if(WIN32)
        # ws2_32 # network
        target_link_libraries(${APP_FLECS_NAME1} PRIVATE 
            ws2_32                                          # Winsock 2
            gdi32                                           # Graphics Device Interface
            user32                                          # Windows user interface
            shell32                                         # Windows 
            ws2_32                                          # network socket
        )
        # target_link_options(${APP_NAME} PRIVATE -static-libgcc)
        target_link_options(${APP_FLECS_NAME1} PRIVATE
            -static-libgcc                                  # GNU Compiler Collection
            -static-libstdc++                               # Uncomment if C++ code is used
            -static                                         # Avoid full static linking to prevent issues with system libraries
        )
    endif()
endif()

set(EXPORT_BE_APP OFF)
# set(EXPORT_FLECS_APP OFF)
if(${EXPORT_BE_APP})
    message(STATUS "EXPORT LIBEVENT APP")
    set(APP_LEVENT_NAME rl_lev)
    add_executable(${APP_LEVENT_NAME}
        examples/c/raylib_event_base_dispatch.c
    )
    target_compile_definitions(${APP_LEVENT_NAME} PRIVATE 
        # RAYGUI_IMPLEMENTATION=1
    )
    # Link application with custom_cimgui
    target_link_libraries(${APP_LEVENT_NAME} PRIVATE 
        raylib                                          # raylib
        # Libevent::core
        # libevent::core
        event_shared 
        event_core_shared 
        # libevent
        # lua                                           # lua
        # flecs                                           # flecs
        # cglm                                            # cglm
        # ODE                                             # ode must be cap from ode repo cmake config
    )
    # Include directories for the application
    target_include_directories(${APP_LEVENT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include                       # include
        # ${lua_SOURCE_DIR}                                   # lua
        ${raylib_SOURCE_DIR}/src                            # raylib include
        # ${raylib_SOURCE_DIR}/src/external/glfw/include      # glfw
        # ${enet_SOURCE_DIR}/include                          # enet
        # ${stb_SOURCE_DIR}                                 # stb
        ${raygui_SOURCE_DIR}/src                            # raygui
        # ${cglm_SOURCE_DIR}/include                          # clgm
        # ${ode_SOURCE_DIR}/include                         # 
        # ${ode_BINARY_DIR}/include                           # 
        ${libevent_SOURCE_DIR}/include                           # 
        ${libevent_BINARY_DIR}/include                           # event2/event-config.h
    )
    # Windows-specific settings
    if(WIN32)
        # ws2_32 # network
        target_link_libraries(${APP_LEVENT_NAME} PRIVATE 
            ws2_32                                          # Winsock 2
            gdi32                                           # Graphics Device Interface
            user32                                          # Windows user interface
            shell32                                         # Windows 
            ws2_32                                          # network socket
        )
        # target_link_options(${APP_NAME} PRIVATE -static-libgcc)
        target_link_options(${APP_LEVENT_NAME} PRIVATE
            -static-libgcc                                  # GNU Compiler Collection
            -static-libstdc++                               # Uncomment if C++ code is used
            -static                                         # Avoid full static linking to prevent issues with system libraries
        )
    endif()
endif()

set(EXPORT_ENET_APP OFF)
# set(EXPORT_FLECS_APP OFF)
if(${EXPORT_ENET_APP})
    message(STATUS "EXPORT ENET APP")
    set(APP_ENET_CLIENT_NAME enet_client)
    add_executable(${APP_ENET_CLIENT_NAME}
        src/enet_client.c
    )
    # Link application with custom_cimgui
    target_link_libraries(${APP_ENET_CLIENT_NAME} PRIVATE 
        raylib                                          # raylib
        flecs                                           # flecs
        # cglm                                            # cglm
        # ODE                                             # ode must be cap from ode repo cmake config
    )
    # Include directories for the application
    target_include_directories(${APP_ENET_CLIENT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include                       # include
        ${lua_SOURCE_DIR}                                   # lua
        ${raylib_SOURCE_DIR}/src                            # raylib include
        ${raylib_SOURCE_DIR}/src/external/glfw/include      # glfw
        ${enet_SOURCE_DIR}/include                          # enet
        # ${stb_SOURCE_DIR}                                 # stb
        ${raygui_SOURCE_DIR}/src                            # raygui
        ${cglm_SOURCE_DIR}/include                          # clgm
        # ${ode_SOURCE_DIR}/include                         # 
        # ${ode_BINARY_DIR}/include                           # 
    )
    # Windows-specific settings
    if(WIN32)
        target_link_libraries(${APP_ENET_CLIENT_NAME} PRIVATE 
            ws2_32                                          # Winsock 2
            gdi32                                           # Graphics Device Interface
            user32                                          # Windows user interface
            shell32                                         # Windows 
            ws2_32                                          # network socket
        )
        target_link_options(${APP_ENET_CLIENT_NAME} PRIVATE
            -static-libgcc                                  # GNU Compiler Collection
            -static-libstdc++                               # Uncomment if C++ code is used
            -static                                         # Avoid full static linking to prevent issues with system libraries
        )
    endif()
    set(APP_ENET_SERVER_NAME enet_server)
    add_executable(${APP_ENET_SERVER_NAME}
        src/enet_server.c
    )
    # Link application with custom_cimgui
    target_link_libraries(${APP_ENET_SERVER_NAME} PRIVATE 
        raylib                                          # raylib
        flecs                                           # flecs
        # cglm                                            # cglm
        # ODE                                             # ode must be cap from ode repo cmake config
    )
    # Include directories for the application
    target_include_directories(${APP_ENET_SERVER_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include                       # include
        ${lua_SOURCE_DIR}                                   # lua
        ${raylib_SOURCE_DIR}/src                            # raylib include
        ${raylib_SOURCE_DIR}/src/external/glfw/include      # glfw
        ${enet_SOURCE_DIR}/include                          # enet
        # ${stb_SOURCE_DIR}                                 # stb
        ${raygui_SOURCE_DIR}/src                            # raygui
        ${cglm_SOURCE_DIR}/include                          # clgm
        # ${ode_SOURCE_DIR}/include                         # 
        # ${ode_BINARY_DIR}/include                           # 
    )
    # Windows-specific settings
    if(WIN32)
        target_link_libraries(${APP_ENET_SERVER_NAME} PRIVATE 
            ws2_32                                          # Winsock 2
            gdi32                                           # Graphics Device Interface
            user32                                          # Windows user interface
            shell32                                         # Windows 
            ws2_32                                          # network socket
        )
        target_link_options(${APP_ENET_SERVER_NAME} PRIVATE
            -static-libgcc                                  # GNU Compiler Collection
            -static-libstdc++                               # Uncomment if C++ code is used
            -static                                         # Avoid full static linking to prevent issues with system libraries
        )
    endif()
endif()

# set(EXPORT_ODE_APP ON)
set(EXPORT_FLECS_APP OFF)
if(${EXPORT_ODE_APP})
    set(APP_ODE_NAME rl_flecs_ode)
    add_executable(${APP_ODE_NAME}
        # examples/c/raylib_flecs_ode_gui03.c
        # examples/c/raylib_flecs_ode_controller.c
        # examples/c/raylib_ode_static_box.c
        # examples/c/raylib_ode_capsule_mesh.c
        examples/c/raylib_ode_capsule_controller.c
        # examples/c/raylib_physics_ode_simple.c
        # examples/c/raylib_physics_ode_collision_sensor.c
    )
    # Link application with custom_cimgui
    target_link_libraries(${APP_ODE_NAME} PRIVATE 
        raylib                                          # raylib
        flecs                                           # flecs
        # cglm                                            # cglm
        ODE                                             # ode must be cap from ode repo cmake config
    )
    # Include directories for the application
    target_include_directories(${APP_ODE_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include                       # include
        ${lua_SOURCE_DIR}                                   # lua
        ${raylib_SOURCE_DIR}/src                            # raylib include
        ${raylib_SOURCE_DIR}/src/external/glfw/include      # glfw
        ${enet_SOURCE_DIR}/include                          # enet
        # ${stb_SOURCE_DIR}                                 # stb
        ${raygui_SOURCE_DIR}/src                            # raygui
        ${cglm_SOURCE_DIR}/include                          # clgm
        # ${ode_SOURCE_DIR}/include                         # 
        ${ode_BINARY_DIR}/include                           # 
    )
    # Windows-specific settings
    if(WIN32)
        target_link_libraries(${APP_ODE_NAME} PRIVATE 
            ws2_32                                          # Winsock 2
            gdi32                                           # Graphics Device Interface
            user32                                          # Windows user interface
            shell32                                         # Windows 
            ws2_32                                          # network socket
        )
        target_link_options(${APP_ODE_NAME} PRIVATE
            -static-libgcc                                  # GNU Compiler Collection
            -static-libstdc++                               # Uncomment if C++ code is used
            -static                                         # Avoid full static linking to prevent issues with system libraries
        )
    endif()
endif()

set(EXPORT_EXAMPLES_APP OFF) #ON OFF bool
# set(EXPORT_EX_APP ON) #ON OFF bool
if(${EXPORT_EXAMPLES_APP})

    # Define the list of example source files
    set(examples
        # src/main01.c
        # src/main02.c
        # examples/c/raylib_physics_ode.c  # Include your physics test file
        # examples/c/rl_flecs_camera3d_cube_picking03.c  # 
        examples/c/raylib_camera_move.c  # 
    )

    set(SRC_MODULES
        src/raygui_impl.c # define RAYGUI_IMPLEMENTATION
        # src/ecs_components.c
        # src/module_dev.c
        # src/module_enet.c # not there no define...
        # src/module_ode.c
    )

    # Loop over each example to create separate executables
    foreach(example ${examples})
        # Extract the filename without extension to use as the executable name
        get_filename_component(example_name ${example} NAME_WE)
        set(APP_FLECS_NAME "ex_${example_name}")  # e.g., ex_main01, ex_main02

        # Create executable for the current example
        add_executable(${APP_FLECS_NAME} 
            ${SRC_MODULES}
            ${example}
        )

        # target_compile_definitions(${APP_FLECS_NAME} PRIVATE 
        #     RAYGUI_IMPLEMENTATION # src/raygui_impl.c
        # )

        # Link application with libraries
        target_link_libraries(${APP_FLECS_NAME} PRIVATE 
            raylib                                              # raylib
            flecs                                               # flecs
            cglm                                                # cglm
            ODE                                                 # ode
        )

        # Include directories for the application
        target_include_directories(${APP_FLECS_NAME} PUBLIC
            ${PROJECT_SOURCE_DIR}/include                       # include
            ${lua_SOURCE_DIR}                                   # lua
            ${raylib_SOURCE_DIR}/src                            # raylib include
            ${raylib_SOURCE_DIR}/src/external/glfw/include      # glfw
            ${enet_SOURCE_DIR}/include                          # enet
            # ${stb_SOURCE_DIR}                                 # stb
            ${raygui_SOURCE_DIR}/src                            # raygui
            ${cglm_SOURCE_DIR}/include                          # clgm
            # ${ode_SOURCE_DIR}/include                         # 
            ${ode_BINARY_DIR}/include                           # 
        )

        # Windows-specific settings
        if(WIN32)
            target_link_libraries(${APP_FLECS_NAME} PRIVATE 
                ws2_32                                          # Winsock 2
                gdi32                                           # Graphics Device Interface
                user32                                          # Windows user interface
                shell32                                         # Windows 
                ws2_32                                          # network socket
            )
            target_link_options(${APP_FLECS_NAME} PRIVATE
                -static-libgcc                                  # GNU Compiler Collection
                -static-libstdc++                               # Uncommented since you might use C++ with cglm/flecs
                -static                                         # Avoid full static linking
            )
        endif()
    endforeach()

    message(STATUS "EXPORT FLECS APPS")
endif()