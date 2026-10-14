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
    src/main/math/fixed_mul.c
    src/main/math/fixed_div.c
    src/main/math/fixed_round.c
    src/main/math/sqrt_approx.c
    src/main/fieldfx/angle.c
    src/main/psyq/libgte/mtx_000.c
    src/main/psyq/libgte/cmb_06.c
    src/main/anim/bone_rotations.c
    src/main/render/bone_transforms.c
    src/main/render/vertex_projection.c)
target_include_directories(parasite-eve-original PRIVATE include)
target_sources(parasite-eve-pc PRIVATE $<TARGET_OBJECTS:parasite-eve-original>)

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
