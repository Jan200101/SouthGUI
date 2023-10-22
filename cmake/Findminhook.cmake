### Get same Minhook as Northstar

if (minhook_FOUND)
    return()
endif()

find_package(NorthstarPluginABI REQUIRED)

check_init_submodule(${NS_LAUNCHER_DIR}/thirdparty/minhook)
add_subdirectory(${NS_LAUNCHER_DIR}/thirdparty/minhook minhook)

set(minhook_FOUND 1 PARENT_SCOPE)