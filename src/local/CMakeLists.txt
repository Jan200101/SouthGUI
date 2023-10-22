
find_package(spdlog REQUIRED)
find_package(NorthstarPluginABI REQUIRED)
find_package(kiero REQUIRED)
find_package(imgui REQUIRED)

add_executable(LocalGUI
	$<TARGET_OBJECTS:SouthGUI>
	"${CMAKE_CURRENT_SOURCE_DIR}/dx.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/dx.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_include_directories(LocalGUI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_precompile_headers(LocalGUI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../ns_plugin.h")

target_link_libraries(LocalGUI spdlog::spdlog_header_only)
target_link_libraries(LocalGUI d3d11)
target_link_libraries(LocalGUI kiero imgui)
target_include_directories(LocalGUI PRIVATE ${NS_DLL_DIR})
target_include_directories(LocalGUI PRIVATE ${NS_PLUG_DIR})