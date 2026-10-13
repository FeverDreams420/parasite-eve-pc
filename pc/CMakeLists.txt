cmake_minimum_required(VERSION 3.21..3.31)

project(parasite-eve-pc C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(PSYZ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/psyz)

add_subdirectory(${PSYZ_DIR}/psyz ${CMAKE_CURRENT_BINARY_DIR}/psyz)

add_executable(parasite-eve-pc main.c)
target_link_libraries(parasite-eve-pc PRIVATE psyz)
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
