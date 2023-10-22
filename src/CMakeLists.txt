
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(kiero REQUIRED)
find_package(imgui REQUIRED)

plugin_manifest(SouthGUI name "SouthGUI")
plugin_manifest(SouthGUI displayname "SouthGUI")
plugin_manifest(SouthGUI description "Dear ImGui Plugin for Northstar")
plugin_manifest(SouthGUI run_on_server OFF)
plugin_manifest(SouthGUI run_on_client ON)

add_library(SouthGUI SHARED
    "${CMAKE_CURRENT_SOURCE_DIR}/init.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/helper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/helper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/hook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/hook.h"
)

target_include_directories(SouthGUI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_precompile_headers(SouthGUI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ns_plugin.h")
target_link_libraries(SouthGUI kiero imgui)

plugin_link(SouthGUI)
plugin_thunderstore(SouthGUI
    "https://github.com/Jan200101/SouthGUI"
    "${PROJECT_SOURCE_DIR}/README.md"
    "${PROJECT_SOURCE_DIR}/icon.png"
)

if (BUILD_LOCAL)
    add_subdirectory(local)
endif()
