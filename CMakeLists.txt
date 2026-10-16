cmake_minimum_required(VERSION 3.21..3.31)
project(parasite-eve-pc LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(BUILD_TESTING "Build original game tests" OFF)
include(CTest)

# PsyZ belongs to the original game build, independently of the abandoned frontend.
set(PSYZ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/psyz)
add_subdirectory(${PSYZ_DIR}/psyz ${CMAKE_CURRENT_BINARY_DIR}/psyz)
target_compile_definitions(psyz PRIVATE __psyz)

# Whole original modules may contain entry points not yet called by each test.
# Keep function-level linking instead of splitting source files for test targets.
if(APPLE)
    add_link_options(-Wl,-dead_strip)
elseif(NOT MSVC)
    add_link_options(-Wl,--gc-sections)
endif()

# Original scene/archive module owns file-image I/O and completion flags.
add_library(parasite-eve-original-disc OBJECT src/main/cdrom/scene_assets.c)
target_include_directories(parasite-eve-original-disc PRIVATE include)
if(MSVC)
    target_compile_options(parasite-eve-original-disc PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-disc PRIVATE -ffunction-sections -fdata-sections)
endif()

# The executable starts the original game and compiles its modules in place.
add_executable(parasite-eve-pc src/main/boot/main.c)
target_include_directories(parasite-eve-pc PRIVATE include)
target_link_libraries(parasite-eve-pc PRIVATE psyz)
target_compile_definitions(parasite-eve-pc PRIVATE __psyz)
psyz_title(parasite-eve-pc "Parasite Eve")
psyz_web_configure_target(parasite-eve-pc)
psyz_ios_configure_target(parasite-eve-pc
    BUNDLE_IDENTIFIER "dev.parasiteeve.pc" DISPLAY_NAME "Parasite Eve")

# The original motion module has its own copies of the common class
# methods and immutable tables, just as the original room library did.
add_library(parasite-eve-original-motion OBJECT
    src/overlays/room_lib/motion_particles.c
    src/overlays/room_lib/motion_particles_2.c
    src/overlays/room_lib/motion_particles_3.c
    src/overlays/room_lib/class_plant_script.c
    src/overlays/room_lib/class_register_draw_list.c
    src/overlays/room_lib/class_register_tables.c
    src/overlays/room_lib/class_nop6.c)
target_include_directories(parasite-eve-original-motion PRIVATE include)
target_link_libraries(parasite-eve-original-motion PRIVATE psyz)
target_compile_definitions(parasite-eve-original-motion PRIVATE PE_OVERLAY=motion)
if(MSVC)
    target_compile_options(parasite-eve-original-motion PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-motion PRIVATE -ffunction-sections -fdata-sections)
endif()

# Compile the original game sources in place. Modules join this target as
# their platform dependencies are made portable; no duplicate game logic.
add_library(parasite-eve-original OBJECT
    src/main/battle/encounter.c
    src/main/battle/setup.c
    src/main/battle/turn.c
    src/main/battle/enemy_actions.c
    src/main/battle/status_hud.c
    src/main/battle/attack.c
    src/main/battle/context.c
    src/main/battle/player_turn.c
    src/main/battle/phase.c
    src/main/battle/enemy_slots.c
    src/main/battle/enemy_movement.c
    src/main/entity/combat.c
    src/main/aya/stats.c
    src/main/item/active_list.c
    src/main/item/actions.c
    src/main/item/swap.c
    src/main/menu/skills.c
    src/main/menu/equip_screen.c
    src/main/menu/equip_select.c
    src/main/menu/item_actions.c
    src/main/menu/item_use_panel.c
    src/main/menu/main_menu.c
    src/main/menu/main_update.c
    src/main/time/counters.c
    src/main/psyq/libsnd/ssinit_c.c
    src/main/save/game_data.c
    src/main/memcard/save_flow.c
    src/main/menu/inventory.c
    src/main/item/armor_capacity.c
    src/main/render/font.c
    src/main/render/font_load.c
    src/main/menu/status_panels.c
    src/main/menu/save_screens.c
    src/main/menu/save_slot.c
    src/main/menu/lists.c
    src/main/menu/item_detail.c
    src/main/battle/commands.c
    src/main/text/tables.c
    src/main/asset/tim_textures.c
    src/main/asset/unload_table.c
    src/main/math/fixed_mul.c
    src/main/math/fixed_div.c
    src/main/math/fixed_round.c
    src/main/math/sqrt_approx.c
    src/main/fieldfx/angle.c
    src/main/psyq/libgte/reg03.c
    src/main/psyq/libgte/fog_02.c
    src/main/psyq/libgte/mtx_000.c
    src/main/psyq/libgte/mtx_001.c
    src/main/psyq/libgte/msc06.c
    src/main/psyq/libgte/mtx_06.c
    src/main/psyq/libgte/cmb_06.c
    src/main/anim/bone_rotations.c
    src/main/render/object.c
    src/main/render/wireframe.c
    src/main/render/frame_update.c
    src/main/entity/frame_update.c
    src/main/boot/init.c
    src/main/boot/frame_loop.c
    src/main/draw/pipeline.c
    src/main/draw/gpu_regs.c
    src/main/draw/prim_queue.c
    src/main/draw/prim_color.c
    src/main/draw/frame.c
    src/main/draw/text.c
    src/main/draw/textbox.c
    src/overlays/menu_memcard/list_screen.c
    src/overlays/menu_memcard/video_playback.c
    src/overlays/menu_memcard/video.c
    src/overlays/menu_memcard/glint_effects.c
    src/overlays/menu_memcard/image_effects.c
    src/overlays/mdec_diag/vlc_build.c
    src/overlays/mdec_diag/vlc_c.c
    src/overlays/menu_memcard/page_flip.c
    src/overlays/menu_memcard/image_nodes.c
    src/overlays/menu_memcard/fade.c
    src/main/draw/wipe.c
    src/main/draw/color_quad.c
    src/main/pm/disc_tables.c
    src/main/pm/process.c
    src/main/entity/pool.c
    src/main/entity/allocation.c
    src/main/script/node_pool.c
    src/main/script/sfx_queue.c
    src/main/psyq/libds/dssys_3.c
    src/main/script/node_actions.c
    src/main/script/actor_opcodes.c
    src/main/script/value_ops.c
    src/main/script/flow_opcodes.c
    src/main/script/system_opcodes.c
    src/overlays/fx_common/effects.c
    src/overlays/fx_common/effect_init.c
    src/overlays/fx_common/motion_endpoints.c
    src/overlays/fx_common/camera_path.c
    src/overlays/fx_common/camera.c
    src/overlays/fx_common/model_buffers.c
    src/overlays/boot_display/text_screen.c
    src/overlays/boot_display/playback_file.c
    src/overlays/boot_display/playback.c
    src/overlays/boot_display/playback_stop.c
    src/overlays/boot_display/image_display.c
    src/overlays/boot_display/sprites.c
    src/main/fieldfx/geometry.c
    src/main/fieldfx/model.c
    src/main/fieldfx/sprite_render.c
    src/main/fieldfx/sprite_effects.c
    src/main/fieldfx/collision.c
    src/main/fieldfx/glow.c
    src/main/fieldfx/burst.c
    src/main/fieldfx/object_script.c
    src/main/fieldfx/task_list.c
    src/main/fieldfx/vector.c
    src/main/field/actor_collision.c
    src/overlays/room_lib/effect_sequence.c
    src/overlays/room_lib/class_nop0.c
    src/overlays/room_lib/class_plant_script.c
    src/overlays/room_lib/glow_orb.c
    src/overlays/room_lib/drop_field.c
    src/overlays/room_lib/glow_orb_setup.c
    src/overlays/room_lib/class_spawn.c
    src/overlays/room_lib/class_close_target.c
    src/overlays/room_lib/class_register_active.c
    src/overlays/room_lib/actor_classes.c
    src/overlays/room_lib/actor_class_table.c
    src/overlays/room_lib/window_class.c
    src/overlays/room_lib/window_class_table.c
    src/overlays/room_m272_liberty/m272.c
    src/overlays/room_m005_theater/m005.c
    src/overlays/room_m023_theater/m023.c
    src/overlays/room_lib/script_task_var.c
    src/overlays/room_lib/orbit_flare.c
    src/overlays/room_lib/script_args.c)
target_include_directories(parasite-eve-original PRIVATE include)
target_link_libraries(parasite-eve-original PRIVATE psyz)
set_property(SOURCE src/main/boot/init.c APPEND PROPERTY INCLUDE_DIRECTORIES
    "$<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>")
if(MSVC)
    target_compile_options(parasite-eve-original PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original PRIVATE -ffunction-sections -fdata-sections)
endif()
target_sources(parasite-eve-pc PRIVATE $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)

# Original AKAO units compile in place while their SPU/timer dependencies
# are connected. The title frontend uses the original effect playback path.
add_library(parasite-eve-original-akao OBJECT
    src/main/cdrom/track_read.c
    src/main/akao/voice_driver.c
    src/main/akao/voice_bank.c
    src/main/akao/sequencer.c
    src/main/akao/spu_transfer.c
    src/main/psyq/libspu/s_svva.c)
target_include_directories(parasite-eve-original-akao PRIVATE include)
target_link_libraries(parasite-eve-original-akao PRIVATE psyz)
if(MSVC)
    target_compile_options(parasite-eve-original-akao PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-akao PRIVATE -ffunction-sections -fdata-sections)
endif()

set(akao_runtime_targets parasite-eve-pc)
foreach(akao_target IN LISTS akao_runtime_targets)
    target_sources(${akao_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original-actor-animation>
        $<TARGET_OBJECTS:parasite-eve-original-akao>
        $<TARGET_OBJECTS:parasite-eve-original-shadow>
        $<TARGET_OBJECTS:parasite-eve-original-disc>)
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
endif()

if(BUILD_TESTING)
    add_executable(test-original-math tests/original_math.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-math PRIVATE include)
    target_link_libraries(test-original-math PRIVATE psyz)
    add_test(NAME original-game-math COMMAND test-original-math)
endif()

if(BUILD_TESTING)
    add_executable(test-original-gte tests/original_gte.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-gte PRIVATE include)
    target_link_libraries(test-original-gte PRIVATE psyz)
    add_test(NAME original-game-gte COMMAND test-original-gte)
endif()

if(BUILD_TESTING)
    add_executable(test-original-animation tests/original_animation.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-animation PRIVATE include)
    target_link_libraries(test-original-animation PRIVATE psyz)
    add_test(NAME original-game-animation COMMAND test-original-animation)
endif()

if(BUILD_TESTING)
    add_executable(test-original-transforms tests/original_transforms.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-transforms PRIVATE include)
    target_link_libraries(test-original-transforms PRIVATE psyz)
    add_test(NAME original-game-transforms COMMAND test-original-transforms)
endif()

if(BUILD_TESTING)
    add_executable(test-original-projection tests/original_projection.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-projection PRIVATE include)
    target_link_libraries(test-original-projection PRIVATE psyz)
    add_test(NAME original-game-projection COMMAND test-original-projection)
endif()

if(BUILD_TESTING)
    add_executable(test-original-hierarchy tests/original_hierarchy.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-hierarchy PRIVATE include)
    target_link_libraries(test-original-hierarchy PRIVATE psyz)
    add_test(NAME original-game-hierarchy COMMAND test-original-hierarchy)
endif()

if(BUILD_TESTING)
    add_executable(test-original-input-rules tests/original_input_rules.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-input-rules PRIVATE include)
    target_link_libraries(test-original-input-rules PRIVATE psyz)
    add_test(NAME original-game-input-rules COMMAND test-original-input-rules)
endif()

if(BUILD_TESTING)
    add_executable(test-original-pools tests/original_pools.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-pools PRIVATE include)
    target_link_libraries(test-original-pools PRIVATE psyz)
    add_test(NAME original-game-pools COMMAND test-original-pools)
endif()

if(BUILD_TESTING)
    add_executable(test-original-nodes tests/original_nodes.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-nodes PRIVATE include)
    target_link_libraries(test-original-nodes PRIVATE psyz)
    add_test(NAME original-game-nodes COMMAND test-original-nodes)
endif()

if(BUILD_TESTING)
    add_executable(test-original-queue tests/original_queue.c
        $<TARGET_OBJECTS:parasite-eve-original-actor-animation>
        $<TARGET_OBJECTS:parasite-eve-original-disc>
        $<TARGET_OBJECTS:parasite-eve-original-akao>
        $<TARGET_OBJECTS:parasite-eve-original-camera>
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-queue PRIVATE include)
    target_link_libraries(test-original-queue PRIVATE psyz)
    add_test(NAME original-game-queue COMMAND test-original-queue)
endif()

if(BUILD_TESTING)
    add_executable(test-original-expression tests/original_expression.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-expression PRIVATE include)
    target_link_libraries(test-original-expression PRIVATE psyz)
    add_test(NAME original-game-expression COMMAND test-original-expression)
endif()

if(BUILD_TESTING)
    add_executable(test-original-contact tests/original_contact.c
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-contact PRIVATE include)
    target_link_libraries(test-original-contact PRIVATE psyz)
    add_test(NAME original-game-contact COMMAND test-original-contact)
endif()

if(BUILD_TESTING)
    add_executable(test-original-movement tests/original_movement.c
        src/main/asset/game_state.c
        $<TARGET_OBJECTS:parasite-eve-original-disc>
        $<TARGET_OBJECTS:parasite-eve-original-akao>
        $<TARGET_OBJECTS:parasite-eve-original-camera>
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-movement PRIVATE include)
    target_link_libraries(test-original-movement PRIVATE psyz)
    add_test(NAME original-game-movement COMMAND test-original-movement)
endif()

add_library(parasite-eve-original-actor-animation OBJECT
    src/main/asset/game_state.c src/main/scene/player_movement.c src/main/psyq/libgte/mtx_004.c)
target_include_directories(parasite-eve-original-actor-animation PRIVATE include)
if(MSVC)
    target_compile_options(parasite-eve-original-actor-animation PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-actor-animation PRIVATE -ffunction-sections -fdata-sections)
endif()
if(BUILD_TESTING)
    add_executable(test-original-actor-animation tests/original_actor_animation.c $<TARGET_OBJECTS:parasite-eve-test-disc>
        $<TARGET_OBJECTS:parasite-eve-original-akao>
        $<TARGET_OBJECTS:parasite-eve-original-camera>
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>
        $<TARGET_OBJECTS:parasite-eve-original-actor-animation>)
    target_include_directories(test-original-actor-animation PRIVATE include)
    target_link_libraries(test-original-actor-animation PRIVATE psyz)
    add_test(NAME original-game-actor-animation COMMAND test-original-actor-animation)
    add_test(NAME original-game-glint COMMAND test-original-actor-animation --glint)
    add_test(NAME original-game-fx-camera COMMAND test-original-actor-animation --fx-camera)
    add_test(NAME original-game-fx-display COMMAND test-original-actor-animation --fx-display)
    add_test(NAME original-game-fx-effects COMMAND test-original-actor-animation --fx-effects)
    add_test(NAME original-game-fx-scene COMMAND test-original-actor-animation --fx-scene)
    set_tests_properties(original-game-fx-scene PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    add_test(NAME original-game-fx-step COMMAND test-original-actor-animation --fx-step)
    set_tests_properties(original-game-fx-step PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    add_test(NAME original-game-credits-text COMMAND test-original-actor-animation --credits-text)
    set_tests_properties(original-game-credits-text PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60 LABELS "game-assets")
    add_test(NAME original-game-credits-images COMMAND test-original-actor-animation --credits-images)
    set_tests_properties(original-game-credits-images PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60 LABELS "game-assets;gpu")
    add_test(NAME original-game-credits-sprites COMMAND test-original-actor-animation --credits-sprites)
    set_tests_properties(original-game-credits-sprites PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60 LABELS "game-assets;gpu")
    add_test(NAME original-game-credits-playback COMMAND test-original-actor-animation --credits-playback)
    set_tests_properties(original-game-credits-playback PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 90 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-texture-pages COMMAND test-original-actor-animation --texture-pages)
    set_tests_properties(original-game-texture-pages PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60 LABELS "game-assets;gpu")
    add_test(NAME original-game-bootstrap-cli COMMAND parasite-eve-pc --help)
    add_test(NAME original-game-bootstrap-arguments COMMAND parasite-eve-pc --frame-limit invalid)
    set_tests_properties(original-game-bootstrap-arguments PROPERTIES WILL_FAIL TRUE)
    add_test(NAME original-game-bootstrap COMMAND test-original-actor-animation --native-bootstrap $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-bootstrap PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 180 LABELS "game-assets;gpu")
    add_test(NAME original-game-field-control COMMAND test-original-actor-animation --native-field-control $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-field-control PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 480 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-stage-transition COMMAND test-original-actor-animation --native-stage $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-stage-transition PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 180 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-battle-entry COMMAND test-original-actor-animation --native-battle-entry $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-battle-entry PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 180 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-melissa-encounter COMMAND test-original-actor-animation --native-melissa $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-melissa-encounter PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 240 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-backstage-transition COMMAND test-original-actor-animation --native-backstage $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-backstage-transition PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 300 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-hallway-transition COMMAND test-original-actor-animation --native-hallway $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-hallway-transition PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 300 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-rat-encounter COMMAND test-original-actor-animation --native-rat $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-rat-encounter PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 300 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-m020-items COMMAND test-original-actor-animation --native-m020-items $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-m020-items PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 360 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-m017-survivor COMMAND test-original-actor-animation --native-m017 $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-m017-survivor PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 420 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-m028-sewer COMMAND test-original-actor-animation --native-m028 $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-m028-sewer PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 720 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-m023-melissa COMMAND test-original-actor-animation --native-m023 $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-m023-melissa PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 600 LABELS "game-assets;gpu;audio")
    add_test(NAME original-game-m022-keys COMMAND test-original-actor-animation --native-m022 $<TARGET_FILE:parasite-eve-pc>)
    set_tests_properties(original-game-m022-keys PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 480 LABELS "game-assets;gpu;audio")
    add_dependencies(test-original-actor-animation parasite-eve-pc)
    add_test(NAME original-game-fx-frames COMMAND test-original-actor-animation --fx-frames)
    set_tests_properties(original-game-fx-frames PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 120 LABELS "game-assets;gpu")
    add_test(NAME original-game-fx-motion COMMAND test-original-actor-animation --fx-motion)
    set_tests_properties(original-game-fx-motion PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    add_test(NAME original-game-fx-models COMMAND test-original-actor-animation --fx-models)
    set_tests_properties(original-game-fx-models PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
    add_test(NAME original-game-player-input-assets COMMAND test-original-actor-animation --assets)
    set_tests_properties(original-game-player-input-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

if(BUILD_TESTING)
    add_executable(test-original-floor tests/original_floor.c $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-floor PRIVATE include)
    target_link_libraries(test-original-floor PRIVATE psyz)
    add_test(NAME original-game-floor COMMAND test-original-floor)
endif()

add_library(parasite-eve-original-camera OBJECT src/main/render/camera.c)
target_include_directories(parasite-eve-original-camera PRIVATE include)
target_sources(parasite-eve-pc PRIVATE $<TARGET_OBJECTS:parasite-eve-original-camera>)
if(MSVC)
    target_compile_options(parasite-eve-original-camera PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-camera PRIVATE -ffunction-sections -fdata-sections)
endif()
if(BUILD_TESTING)
    add_executable(test-original-camera tests/original_camera.c $<TARGET_OBJECTS:parasite-eve-original-disc>
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>
        $<TARGET_OBJECTS:parasite-eve-original-camera>)
    target_include_directories(test-original-camera PRIVATE include)
    target_link_libraries(test-original-camera PRIVATE psyz)
    add_test(NAME original-game-camera COMMAND test-original-camera)
    set_tests_properties(original-game-camera PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

add_library(parasite-eve-original-pad OBJECT src/main/menu/widget.c src/main/menu/input_queue.c)
target_include_directories(parasite-eve-original-pad PRIVATE include)
target_link_libraries(parasite-eve-original-pad PRIVATE psyz)
foreach(pad_target IN LISTS akao_runtime_targets)
    target_sources(${pad_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original-pad>)
    if(NOT pad_target STREQUAL "parasite-eve-pc")
        target_sources(${pad_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    endif()
endforeach()
if(BUILD_TESTING)
    target_sources(test-original-actor-animation PRIVATE $<TARGET_OBJECTS:parasite-eve-original-pad>)
endif()

if(BUILD_TESTING)
    add_executable(test-original-actor-lifecycle tests/original_actor_lifecycle.c
        $<TARGET_OBJECTS:parasite-eve-original-akao>
        $<TARGET_OBJECTS:parasite-eve-original-camera>
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>
        $<TARGET_OBJECTS:parasite-eve-original-disc>)
    target_include_directories(test-original-actor-lifecycle PRIVATE include)
    target_link_libraries(test-original-actor-lifecycle PRIVATE psyz)
    if(MSVC)
        target_link_options(test-original-actor-lifecycle PRIVATE /OPT:REF)
    elseif(APPLE)
        target_link_options(test-original-actor-lifecycle PRIVATE -Wl,-dead_strip)
    else()
        target_compile_options(test-original-actor-lifecycle PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(test-original-actor-lifecycle PRIVATE -Wl,--gc-sections)
    endif()
    add_test(NAME original-game-actor-lifecycle COMMAND test-original-actor-lifecycle)
endif()

# Original area-bank state machine; native I/O binding is still pending.
add_library(parasite-eve-original-entity-textures OBJECT src/main/scene/room_load.c)
target_include_directories(parasite-eve-original-entity-textures PRIVATE include)
target_sources(parasite-eve-pc PRIVATE $<TARGET_OBJECTS:parasite-eve-original-entity-textures>)
if(BUILD_TESTING)
    target_sources(test-original-actor-animation PRIVATE $<TARGET_OBJECTS:parasite-eve-original-entity-textures>)
endif()
if(MSVC)
    target_compile_options(parasite-eve-original-entity-textures PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-entity-textures PRIVATE -ffunction-sections -fdata-sections)
endif()

if(BUILD_TESTING)
    # Compile the whole module; only this fixture replaces read/poll outcomes.
    # Rename its real I/O symbols to keep them distinct from the fixture doubles.
    add_library(parasite-eve-test-disc OBJECT src/main/cdrom/scene_assets.c)
    target_include_directories(parasite-eve-test-disc PRIVATE include)
    target_compile_definitions(parasite-eve-test-disc PRIVATE
        CdRom_ReadSectorsFromLba=CdRom_ImageReadSectorsFromLba
        CdRom_PollReady=CdRom_ImagePollReady
        Asset_Find08w=Asset_ImageFind08w)
    if(MSVC)
        target_compile_options(parasite-eve-test-disc PRIVATE /Gy /Gw)
    else()
        target_compile_options(parasite-eve-test-disc PRIVATE -ffunction-sections -fdata-sections)
    endif()

endif()

add_library(parasite-eve-original-shadow OBJECT src/overlays/room_lib/mirror_model.c src/main/render/mesh_list.c src/main/render/mesh_entry.c)
target_include_directories(parasite-eve-original-shadow PRIVATE include)
target_compile_definitions(parasite-eve-original-shadow PRIVATE __psyz)
target_link_libraries(parasite-eve-original-shadow PRIVATE psyz)
if(MSVC)
    target_compile_options(parasite-eve-original-shadow PRIVATE /Gy /Gw)
else()
    target_compile_options(parasite-eve-original-shadow PRIVATE -ffunction-sections -fdata-sections)
endif()
if(BUILD_TESTING)
    # Resident script commands can open the original menu and render transition.
    # Link whole existing modules for every fixture that uses the real table.
    foreach(script_target test-original-queue test-original-movement test-original-actor-animation test-original-camera)
        target_sources(${script_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original-shadow>)
    endforeach()
    foreach(script_target test-original-queue test-original-movement)
        target_sources(${script_target} PRIVATE $<TARGET_OBJECTS:parasite-eve-original-pad>
            $<TARGET_OBJECTS:parasite-eve-original-entity-textures>)
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(test-original-shadow tests/original_shadow.c
        $<TARGET_OBJECTS:parasite-eve-original-shadow>
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion>)
    target_include_directories(test-original-shadow PRIVATE include)
    target_compile_definitions(test-original-shadow PRIVATE __psyz)
    target_link_libraries(test-original-shadow PRIVATE psyz)
    add_test(NAME original-game-shadow COMMAND test-original-shadow)
endif()

if(BUILD_TESTING)
    add_executable(test-original-stat-assets tests/original_stat_assets.c $<TARGET_OBJECTS:parasite-eve-original-disc>
        $<TARGET_OBJECTS:parasite-eve-original-camera> $<TARGET_OBJECTS:parasite-eve-original-entity-textures>
        $<TARGET_OBJECTS:parasite-eve-original> $<TARGET_OBJECTS:parasite-eve-original-motion> $<TARGET_OBJECTS:parasite-eve-original-pad>
        $<TARGET_OBJECTS:parasite-eve-original-actor-animation> $<TARGET_OBJECTS:parasite-eve-original-akao>)
    target_include_directories(test-original-stat-assets PRIVATE include)
    target_link_libraries(test-original-stat-assets PRIVATE psyz)
    add_test(NAME original-game-stat-assets COMMAND test-original-stat-assets)
    set_tests_properties(original-game-stat-assets PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 60)
endif()

# Original-module integration runs without creating a window or GPU device.
if(BUILD_TESTING)
    get_property(original_tests DIRECTORY PROPERTY TESTS)
    foreach(original_test IN LISTS original_tests)
        if(original_test MATCHES "^original-game-")
            set_property(TEST ${original_test} APPEND PROPERTY ENVIRONMENT
                "PSYZ_HEADLESS=1" "SDL_AUDIODRIVER=dummy")
        endif()
    endforeach()
endif()
