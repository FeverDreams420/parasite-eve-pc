cmake_minimum_required(VERSION 3.21..3.31)
project(parasite-eve-pc LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(CTest)
add_subdirectory(pc)

# Compile the original game sources in place. Modules join this target as
# their platform dependencies are made portable; no duplicate game logic.
add_library(parasite-eve-original OBJECT
    src/main/pm/scene_init.c
    src/main/math/fixed_mul.c
    src/main/math/fixed_div.c
    src/main/math/fixed_round.c
    src/main/math/sqrt_approx.c
    src/main/fieldfx/angle.c
    src/main/psyq/libgte/mtx_000.c
    src/main/psyq/libgte/cmb_06.c
    src/main/anim/bone_rotations.c
    src/main/render/bone_transforms.c
    src/main/render/vertex_projection.c
    src/main/entity/hierarchy.c
    src/main/scene/input_rules.c
    src/main/entity/pool.c
    src/main/entity/allocation.c
    src/main/entity/block_alloc.c
    src/main/script/node_pool.c
    src/main/script/expression.c
    src/main/script/basic_flow.c
    src/main/field/actor_collision.c)
target_include_directories(parasite-eve-original PRIVATE include)
target_sources(parasite-eve-pc PRIVATE $<TARGET_OBJECTS:parasite-eve-original>)

# Original AKAO units compile in place while their SPU/timer dependencies
# are connected. The title frontend uses the original effect playback path.
add_library(parasite-eve-original-akao OBJECT
    src/main/akao/menu_sound.c
    src/main/cdrom/asset_lookup.c
    src/main/akao/state.c
    src/main/akao/opcode_dispatch.c
    src/main/akao/voice_driver.c
    src/main/akao/voice_bank.c
    src/main/akao/sequencer.c
    src/main/akao/spu_transfer.c
    src/main/psyq/libspu/s_svva.c)
target_include_directories(parasite-eve-original-akao PRIVATE include)
if(MSVC)
    target_compile_options(parasite-eve-original-akao PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-akao PRIVATE -ffunction-sections -fdata-sections)
endif()

set(akao_runtime_targets parasite-eve-pc)
if(BUILD_TESTING)
    list(APPEND akao_runtime_targets test-boot-assets test-boot-replay)
endif()
foreach(akao_target IN LISTS akao_runtime_targets)
    target_sources(${akao_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original-akao>
        src/main/akao/assets.c)
    target_include_directories(${akao_target} PRIVATE include)
    if(MSVC)
        target_link_options(${akao_target} PRIVATE /OPT:REF)
    elseif(APPLE)
        target_link_options(${akao_target} PRIVATE -Wl,-dead_strip)
    else()
        target_link_options(${akao_target} PRIVATE -Wl,--gc-sections)
    endif()
endforeach()

if(BUILD_TESTING)
    add_executable(test-original-akao-layout tests/original_akao_layout.c)
    target_include_directories(test-original-akao-layout PRIVATE include)
    add_test(NAME original-game-akao-layout COMMAND test-original-akao-layout)
    add_executable(test-original-akao-instrument tests/original_akao_instrument.c
        pc/disc.c pc/room.c pc/akao_sequence.c pc/spu_bank.c
        $<TARGET_OBJECTS:parasite-eve-original-akao>)
    target_include_directories(test-original-akao-instrument PRIVATE include pc)
    target_link_libraries(test-original-akao-instrument PRIVATE psyz)
    # Isolate original procedures under test. The fixture rejects any request
    # to the missing reverb-mode endpoint. Full audio must resolve it properly.
    if(MSVC)
        target_link_options(test-original-akao-instrument PRIVATE /OPT:REF)
    elseif(APPLE)
        target_link_options(test-original-akao-instrument PRIVATE -Wl,-dead_strip)
    else()
        target_link_options(test-original-akao-instrument PRIVATE -Wl,--gc-sections)
    endif()
    add_test(NAME original-game-akao-instrument COMMAND test-original-akao-instrument)
    add_test(NAME original-game-akao-assets COMMAND test-original-akao-instrument --assets)
    set_tests_properties(original-game-akao-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    add_test(NAME original-game-akao-menu-assets COMMAND test-original-akao-instrument --menu-assets)
    set_tests_properties(original-game-akao-menu-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    add_test(NAME original-game-akao-tick-assets COMMAND test-original-akao-instrument --tick-assets)
    set_tests_properties(original-game-akao-tick-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    add_test(NAME original-game-akao-prefix-assets COMMAND test-original-akao-instrument --execute-assets)
    set_tests_properties(original-game-akao-prefix-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

if(BUILD_TESTING)
    add_executable(test-original-math tests/original_math.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-math PRIVATE include)
    target_link_libraries(test-original-math PRIVATE psyz)
    add_test(NAME original-game-math COMMAND test-original-math)
endif()

if(BUILD_TESTING)
    add_executable(test-original-gte tests/original_gte.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-gte PRIVATE include)
    target_link_libraries(test-original-gte PRIVATE psyz)
    add_test(NAME original-game-gte COMMAND test-original-gte)
endif()

if(BUILD_TESTING)
    add_executable(test-original-animation tests/original_animation.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-animation PRIVATE include)
    target_link_libraries(test-original-animation PRIVATE psyz)
    add_test(NAME original-game-animation COMMAND test-original-animation)
endif()

if(BUILD_TESTING)
    # Disc I/O and the earlier decoder are test fixtures only. The function
    # under test is the original game procedure linked into the executable.
    add_executable(test-original-animation-assets tests/original_animation_assets.c
        pc/disc.c pc/room.c pc/room_banks.c pc/animation.c pc/matrix.c pc/model.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-animation-assets PRIVATE include pc)
    target_link_libraries(test-original-animation-assets PRIVATE psyz)
    add_test(NAME original-game-animation-assets COMMAND test-original-animation-assets)
    set_tests_properties(original-game-animation-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

if(BUILD_TESTING)
    add_executable(test-original-transforms tests/original_transforms.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-transforms PRIVATE include)
    target_link_libraries(test-original-transforms PRIVATE psyz)
    add_test(NAME original-game-transforms COMMAND test-original-transforms)
endif()

if(BUILD_TESTING)
    add_executable(test-original-projection tests/original_projection.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-projection PRIVATE include)
    target_link_libraries(test-original-projection PRIVATE psyz)
    add_test(NAME original-game-projection COMMAND test-original-projection)
endif()

if(BUILD_TESTING)
    add_executable(test-original-projection-assets tests/original_projection_assets.c
        pc/disc.c pc/room.c pc/room_banks.c pc/projection.c pc/camera.c
        pc/model.c pc/matrix.c $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-projection-assets PRIVATE include pc)
    target_link_libraries(test-original-projection-assets PRIVATE psyz)
    add_test(NAME original-game-projection-assets COMMAND test-original-projection-assets)
    set_tests_properties(original-game-projection-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

if(BUILD_TESTING)
    add_executable(test-original-hierarchy tests/original_hierarchy.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-hierarchy PRIVATE include)
    target_link_libraries(test-original-hierarchy PRIVATE psyz)
    add_test(NAME original-game-hierarchy COMMAND test-original-hierarchy)
endif()

if(BUILD_TESTING)
    add_executable(test-original-input-rules tests/original_input_rules.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-input-rules PRIVATE include)
    target_link_libraries(test-original-input-rules PRIVATE psyz)
    add_test(NAME original-game-input-rules COMMAND test-original-input-rules)
endif()

if(BUILD_TESTING)
    add_executable(test-original-pools tests/original_pools.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-pools PRIVATE include)
    target_link_libraries(test-original-pools PRIVATE psyz)
    add_test(NAME original-game-pools COMMAND test-original-pools)
endif()

if(BUILD_TESTING)
    add_executable(test-original-nodes tests/original_nodes.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-nodes PRIVATE include)
    target_link_libraries(test-original-nodes PRIVATE psyz)
    add_test(NAME original-game-nodes COMMAND test-original-nodes)
endif()

add_library(parasite-eve-original-model-setup OBJECT src/main/render/model_setup.c src/main/render/model_draw.c src/main/render/model_lighting.c src/main/render/texture_upload.c src/main/render/object.c)
target_include_directories(parasite-eve-original-model-setup PRIVATE include)
target_link_libraries(parasite-eve-original-model-setup PRIVATE psyz)
target_compile_definitions(parasite-eve-original-model-setup PRIVATE __psyz)
if(BUILD_TESTING)
    add_executable(test-original-model-setup tests/original_model_setup.c
        pc/disc.c pc/room.c pc/room_banks.c pc/model.c pc/vram.c
        $<TARGET_OBJECTS:parasite-eve-original-model-setup>
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-model-setup PRIVATE include pc)
    target_compile_definitions(test-original-model-setup PRIVATE __psyz)
    target_link_libraries(test-original-model-setup PRIVATE psyz)
    add_test(NAME original-game-model-setup COMMAND test-original-model-setup)
    add_test(NAME original-game-model-gpu COMMAND test-original-model-setup --gpu)
    add_test(NAME original-game-model-room-gpu COMMAND test-original-model-setup --room-gpu)
    set_tests_properties(original-game-model-room-gpu PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120 LABELS "gpu;game-assets")
    set_tests_properties(original-game-model-gpu PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120 LABELS "gpu;game-assets")
    set_tests_properties(original-game-model-setup PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

add_library(parasite-eve-original-queue OBJECT src/main/script/queue.c)
target_include_directories(parasite-eve-original-queue PRIVATE include)
if(BUILD_TESTING)
    add_executable(test-original-queue tests/original_queue.c
        $<TARGET_OBJECTS:parasite-eve-original>
        $<TARGET_OBJECTS:parasite-eve-original-queue>)
    target_include_directories(test-original-queue PRIVATE include)
    target_link_libraries(test-original-queue PRIVATE psyz)
    add_test(NAME original-game-queue COMMAND test-original-queue)
endif()

if(BUILD_TESTING)
    add_executable(test-original-expression tests/original_expression.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-expression PRIVATE include)
    target_link_libraries(test-original-expression PRIVATE psyz)
    add_test(NAME original-game-expression COMMAND test-original-expression)
endif()

# Compile the whole original actor frame module while its remaining game
# dependencies are being ported. It is not linked with replacement stubs.
add_library(parasite-eve-original-frame OBJECT src/main/entity/frame_update.c)
target_include_directories(parasite-eve-original-frame PRIVATE include)

if(BUILD_TESTING)
    add_executable(test-original-contact tests/original_contact.c
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-contact PRIVATE include)
    target_link_libraries(test-original-contact PRIVATE psyz)
    add_test(NAME original-game-contact COMMAND test-original-contact)
endif()

add_library(parasite-eve-original-movement OBJECT src/main/entity/movement.c)
target_include_directories(parasite-eve-original-movement PRIVATE include)
if(BUILD_TESTING)
    add_executable(test-original-movement tests/original_movement.c
        $<TARGET_OBJECTS:parasite-eve-original>
        $<TARGET_OBJECTS:parasite-eve-original-queue>
        $<TARGET_OBJECTS:parasite-eve-original-movement>)
    target_include_directories(test-original-movement PRIVATE include)
    target_link_libraries(test-original-movement PRIVATE psyz)
    add_test(NAME original-game-movement COMMAND test-original-movement)
endif()

add_library(parasite-eve-original-actor-animation OBJECT
    src/main/entity/animation.c src/main/scene/actor_sound.c src/main/scene/player_input.c src/main/psyq/libgte/mtx_004.c)
target_include_directories(parasite-eve-original-actor-animation PRIVATE include)
if(BUILD_TESTING)
    add_executable(test-original-actor-animation tests/original_actor_animation.c pc/disc.c
        $<TARGET_OBJECTS:parasite-eve-original-player-rules>
        $<TARGET_OBJECTS:parasite-eve-original-movement>
        $<TARGET_OBJECTS:parasite-eve-original-queue>
        $<TARGET_OBJECTS:parasite-eve-original>
        $<TARGET_OBJECTS:parasite-eve-original-actor-animation>)
    target_include_directories(test-original-actor-animation PRIVATE include pc)
    target_link_libraries(test-original-actor-animation PRIVATE psyz)
    add_test(NAME original-game-actor-animation COMMAND test-original-actor-animation)
    add_test(NAME original-game-player-input-assets COMMAND test-original-actor-animation --assets)
    set_tests_properties(original-game-player-input-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

if(BUILD_TESTING)
    add_executable(test-original-floor tests/original_floor.c $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-floor PRIVATE include)
    target_link_libraries(test-original-floor PRIVATE psyz)
    add_test(NAME original-game-floor COMMAND test-original-floor)
    add_executable(test-original-collision-assets tests/original_collision_assets.c
        pc/collision.c pc/collision_math.c pc/disc.c pc/room.c $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-collision-assets PRIVATE include pc)
    target_link_libraries(test-original-collision-assets PRIVATE psyz)
    add_test(NAME original-game-collision-assets COMMAND test-original-collision-assets)
    set_tests_properties(original-game-collision-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

add_library(parasite-eve-original-camera OBJECT src/main/render/camera_view.c)
target_include_directories(parasite-eve-original-camera PRIVATE include)
if(BUILD_TESTING)
    add_executable(test-original-camera tests/original_camera.c pc/camera.c pc/disc.c pc/room.c pc/matrix.c pc/model.c
        $<TARGET_OBJECTS:parasite-eve-original-camera>)
    target_include_directories(test-original-camera PRIVATE include pc)
    target_link_libraries(test-original-camera PRIVATE psyz)
    add_test(NAME original-game-camera COMMAND test-original-camera)
    set_tests_properties(original-game-camera PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

add_library(parasite-eve-original-player-rules OBJECT src/main/scene/player_input_rules.c)
target_include_directories(parasite-eve-original-player-rules PRIVATE include)

add_library(parasite-eve-original-pad OBJECT src/main/scene/pad_input.c src/main/menu/widget_lookup.c pc/host_pad.c)
target_include_directories(parasite-eve-original-pad PRIVATE include pc)
target_link_libraries(parasite-eve-original-pad PRIVATE psyz)
foreach(pad_target IN LISTS akao_runtime_targets)
    target_sources(${pad_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original-pad>)
    if(NOT pad_target STREQUAL "parasite-eve-pc")
        target_sources(${pad_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original>)
    endif()
endforeach()
if(BUILD_TESTING)
    target_sources(test-original-actor-animation PRIVATE $<TARGET_OBJECTS:parasite-eve-original-pad>)
endif()

if(BUILD_TESTING)
    add_executable(test-original-actor-lifecycle tests/original_actor_lifecycle.c
        src/main/pm/process.c src/main/pm/frame.c src/main/script/pm_commands.c
        $<TARGET_OBJECTS:parasite-eve-original-camera>
        $<TARGET_OBJECTS:parasite-eve-original>
        $<TARGET_OBJECTS:parasite-eve-original-frame>
        $<TARGET_OBJECTS:parasite-eve-original-model-setup>)
    target_include_directories(test-original-actor-lifecycle PRIVATE include)
    target_link_libraries(test-original-actor-lifecycle PRIVATE psyz)
    if(MSVC)
        target_link_options(test-original-actor-lifecycle PRIVATE /OPT:REF)
    elseif(APPLE)
        target_link_options(test-original-actor-lifecycle PRIVATE -Wl,-dead_strip)
    else()
        target_compile_options(test-original-actor-lifecycle PRIVATE -ffunction-sections -fdata-sections)
        target_compile_options(parasite-eve-original-frame PRIVATE -ffunction-sections -fdata-sections)
        target_compile_options(parasite-eve-original-model-setup PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(test-original-actor-lifecycle PRIVATE -Wl,--gc-sections)
    endif()
    add_test(NAME original-game-actor-lifecycle COMMAND test-original-actor-lifecycle)
endif()

# Original area-bank state machine; native I/O binding is still pending.
add_library(parasite-eve-original-entity-textures OBJECT src/main/scene/entity_textures.c)
target_include_directories(parasite-eve-original-entity-textures PRIVATE include)

if(BUILD_TESTING)
    add_executable(test-original-entity-textures tests/original_entity_textures.c
        $<TARGET_OBJECTS:parasite-eve-original-entity-textures>
        $<TARGET_OBJECTS:parasite-eve-original-camera>
        $<TARGET_OBJECTS:parasite-eve-original-model-setup>
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-entity-textures PRIVATE include pc)
    target_compile_definitions(test-original-entity-textures PRIVATE __psyz)
    target_link_libraries(test-original-entity-textures PRIVATE psyz)
    add_test(NAME original-game-entity-textures COMMAND test-original-entity-textures)
endif()

if(BUILD_TESTING)
    add_executable(test-original-entity-texture-assets tests/original_entity_textures.c
        pc/host_disc.c pc/disc.c pc/room.c pc/vram.c pc/model.c
        src/main/cdrom/asset_lookup.c
        $<TARGET_OBJECTS:parasite-eve-original-entity-textures>
        $<TARGET_OBJECTS:parasite-eve-original-camera>
        $<TARGET_OBJECTS:parasite-eve-original-model-setup>
        $<TARGET_OBJECTS:parasite-eve-original>)
    target_include_directories(test-original-entity-texture-assets PRIVATE include pc)
    target_compile_definitions(test-original-entity-texture-assets PRIVATE __psyz PE_TEST_REAL_DISC)
    target_link_libraries(test-original-entity-texture-assets PRIVATE psyz)
    add_test(NAME original-game-entity-texture-assets COMMAND test-original-entity-texture-assets)
    set_tests_properties(original-game-entity-texture-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120 LABELS "gpu;game-assets")
endif()
