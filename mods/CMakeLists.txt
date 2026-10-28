# Mod host and mods, built only with -DPE_MODS=ON. See mods/README.md.
# Each mod is a folder here holding .c files and defining `const PeMod pe_mod_<folder>`.
# pe_hello is the example mod; add it to the list to build it.
set(PE_MOD_LIST pe_qol pe_rebalance pe_economy pe_collection pe_tools CACHE STRING "Mods built into the game (folder names)")

set(pe_host_sources
    host/pe_mod_host.c      # registry, hooks, services, events, logs
    host/pe_mod_menu.c      # Mods panel rows and pe_mods.ini
    host/pe_mod_panel.c     # the F1 panel: navigation and input
    host/pe_pages.c         # PE Enhancements page layout
    host/pe_game.c          # game access helpers
    host/pe_game_facts.c    # game facts as tables: effect names and groups, power tags
    host/pe_main_menu.c     # extra rows in Aya's main menu, screens from game widgets
    host/pe_equip_sync.c    # edited equipment reaches the next battle
    host/pe_save_tail.c     # the save's free tail carried forward (mods off keep their data)
    host/pe_present.c       # image presenter: pictures pasted into the finished frame
    host/pe_disc_assets.c   # read-only game data from the player's disc (rooms)
    host/pe_model.c         # decoded game models: geometry, textures, idle animation
    host/pe_model_view.c    # a model posed, turned and drawn into a picture
    host/pe_model_stage.c   # a model on show: turning, idle animation, box or full screen
    host/ui/pe_ui_raster.c  # drawing: boxes and text
    host/ui/pe_ui_gpu.c     # the overlay quad
    host/ui/pe_toast.c)     # one-line messages

set(pe_mod_sources ${pe_host_sources})
set(pe_mod_decls "")
set(pe_mod_refs "")
foreach(mod IN LISTS PE_MOD_LIST)
    file(GLOB mod_files CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${mod}/*.c)
    list(APPEND pe_mod_sources ${mod_files})
    string(APPEND pe_mod_decls "extern const PeMod pe_mod_${mod};\n")
    string(APPEND pe_mod_refs "    &pe_mod_${mod},\n")
endforeach()
configure_file(pe_mod_list.c.in ${CMAKE_CURRENT_BINARY_DIR}/pe_mod_list.c @ONLY)

add_library(pe-mods OBJECT ${pe_mod_sources} ${CMAKE_CURRENT_BINARY_DIR}/pe_mod_list.c)
target_include_directories(pe-mods PRIVATE ${PROJECT_SOURCE_DIR}/include host
    "$<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>")
target_compile_definitions(pe-mods PRIVATE __psyz)
target_link_libraries(pe-mods PRIVATE psyz)

if(BUILD_TESTING AND "pe_qol" IN_LIST PE_MOD_LIST)
    add_executable(pe-mod-frequency
        ${PROJECT_SOURCE_DIR}/tests/pe_mod_frequency_test.c
        pe_qol/pe_battle_frequency.c
        pe_qol/pe_move.c)
    target_include_directories(pe-mod-frequency PRIVATE ${PROJECT_SOURCE_DIR}/include host pe_qol
        "$<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>")
    target_compile_definitions(pe-mod-frequency PRIVATE __psyz PE_MODS=1)
    target_link_libraries(pe-mod-frequency PRIVATE psyz)
    add_test(NAME pe-mod-frequency COMMAND pe-mod-frequency)
endif()

if(BUILD_TESTING)
    add_executable(test-mod-settings ${PROJECT_SOURCE_DIR}/tests/original_input_rules.c)
    target_include_directories(test-mod-settings PRIVATE ${PROJECT_SOURCE_DIR}/include host)
    target_compile_definitions(test-mod-settings PRIVATE PE_MOD_SETTINGS_TEST=1 __psyz)
    target_link_libraries(test-mod-settings PRIVATE psyz)
    add_test(NAME mod-settings-save-safety COMMAND test-mod-settings "${CMAKE_CURRENT_BINARY_DIR}/settings-evidence")

    add_executable(test-mod-crash-tail ${PROJECT_SOURCE_DIR}/tests/pe_mod_crash_tail_test.c host/pe_mod_host.c host/pe_save_tail.c)
    target_include_directories(test-mod-crash-tail PRIVATE ${PROJECT_SOURCE_DIR}/include host
        "$<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>")
    target_compile_definitions(test-mod-crash-tail PRIVATE __psyz PE_MODS=1)
    target_link_libraries(test-mod-crash-tail PRIVATE psyz)
    add_test(NAME mod-crash-save-tail COMMAND test-mod-crash-tail)
endif()

if(BUILD_TESTING AND "pe_qol" IN_LIST PE_MOD_LIST)
    add_executable(test-mod-inventory ${PROJECT_SOURCE_DIR}/tests/pe_mod_inventory_test.c
        pe_qol/pe_key_items.c pe_qol/pe_bag_sort.c ${PROJECT_SOURCE_DIR}/src/main/item/armor_capacity.c)
    add_executable(test-mod-inventory-menu ${PROJECT_SOURCE_DIR}/tests/pe_mod_inventory_menu_test.c
        pe_qol/pe_inventory_menu.c)
    foreach(target test-mod-inventory test-mod-inventory-menu)
        target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/include host pe_qol)
        target_compile_definitions(${target} PRIVATE __psyz PE_MODS=1)
        target_link_libraries(${target} PRIVATE psyz)
        if(MSVC)
            target_compile_options(${target} PRIVATE /UNDEBUG)
        else()
            target_compile_options(${target} PRIVATE -UNDEBUG)
        endif()
    endforeach()
    add_test(NAME mod-inventory COMMAND test-mod-inventory)
    add_test(NAME mod-inventory-menu COMMAND test-mod-inventory-menu)
endif()

if(BUILD_TESTING)
    add_executable(test-mod-save-tail ${PROJECT_SOURCE_DIR}/tests/pe_mod_save_tail_test.c host/pe_save_tail.c)
    target_include_directories(test-mod-save-tail PRIVATE ${PROJECT_SOURCE_DIR}/include host)
    target_compile_definitions(test-mod-save-tail PRIVATE __psyz PE_MODS=1)
    target_link_libraries(test-mod-save-tail PRIVATE psyz)
    add_test(NAME mod-preserve-save-tail COMMAND test-mod-save-tail)
endif()
