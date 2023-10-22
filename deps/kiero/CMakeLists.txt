cmake_minimum_required(VERSION 3.21.1)

project(kiero VERSION 1.2.12)

find_package(minhook REQUIRED)

add_library(kiero STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/kiero.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/kiero.h
)

target_link_libraries(kiero PUBLIC minhook)
target_include_directories(kiero PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
