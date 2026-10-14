cmake_minimum_required(VERSION 3.21..3.31)

project(parasite-eve-pc C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(PSYZ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/psyz)

add_subdirectory(${PSYZ_DIR}/psyz ${CMAKE_CURRENT_BINARY_DIR}/psyz)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat libavcodec libavutil libswscale libswresample)

add_executable(parasite-eve-pc main.c disc.c movie.c boot.c title.c)
target_link_libraries(parasite-eve-pc PRIVATE psyz PkgConfig::FFMPEG)
if(MSVC)
    target_compile_options(parasite-eve-pc PRIVATE /W4)
else()
    target_compile_options(parasite-eve-pc PRIVATE -Wall -Wextra -Wpedantic)
endif()
target_include_directories(parasite-eve-pc SYSTEM PRIVATE ${PSYZ_DIR}/psyz/include)
target_compile_definitions(parasite-eve-pc PRIVATE __psyz)
target_compile_definitions(psyz PRIVATE __psyz)

psyz_title(parasite-eve-pc "Parasite Eve")
psyz_web_configure_target(parasite-eve-pc)
psyz_ios_configure_target(parasite-eve-pc
    BUNDLE_IDENTIFIER "dev.parasiteeve.pc"
    DISPLAY_NAME "Parasite Eve")

if(BUILD_TESTING)
    add_executable(test-disc tests/test_disc.c disc.c)
    target_include_directories(test-disc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME disc-reader COMMAND test-disc)
    add_executable(test-boot-assets tests/test_boot_assets.c disc.c movie.c title.c boot.c)
    target_include_directories(test-boot-assets PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(test-boot-assets PRIVATE SDL3::SDL3 PkgConfig::FFMPEG)
    add_test(NAME boot-assets COMMAND test-boot-assets)
    set_tests_properties(boot-assets PROPERTIES SKIP_RETURN_CODE 77
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy" TIMEOUT 15)
endif()
