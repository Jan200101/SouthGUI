
if (kiero_FOUND)
    return()
endif()

find_package(NorthstarPluginABI REQUIRED)

set(KIERO_DIR "${PROJECT_SOURCE_DIR}/deps/kiero" CACHE STRING "Path to kiero dependency")

include_directories("${NS_LAUNCHER_DIR}/thirdparty")
add_subdirectory(${KIERO_DIR} kiero)

set(kiero_FOUND 1)
