cmake_minimum_required(VERSION 3.21.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

project(jukebox VERSION 3.4.0)

file(GLOB SOURCES
    jukebox/jukebox/ui/*.cpp
    jukebox/jukebox/ui/list/*.cpp
    jukebox/jukebox/managers/*.cpp
    jukebox/jukebox/hooks/*.cpp
    jukebox/jukebox/events/*.cpp
    jukebox/jukebox/download/*.cpp
    jukebox/jukebox/utils/*.cpp
    jukebox/jukebox/compat/*.cpp
	jukebox/jukebox/nong/*.cpp
	jukebox/jukebox/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC jukebox)

set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} $ENV{GEODE_SDK}/build)

target_link_libraries(${PROJECT_NAME} geode-sdk)
create_geode_file(${PROJECT_NAME})
