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
    src/main/psyq/libgte/mtx_000.c)
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
