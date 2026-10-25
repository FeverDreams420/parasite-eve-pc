# Mod host and mods, built only with -DPE_MODS=ON.
# Each mod is a folder here holding <id>.c files and defining `const PeMod pe_mod_<id>`.
set(PE_MOD_LIST pe_rebalance pe_economy pe_hello CACHE STRING "Mods built into the game (folder names)")

set(pe_mod_sources host/pe_mod_host.c host/pe_mod_menu.c host/pe_mod_panel.c host/pe_game.c host/pe_ps1.c)
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
