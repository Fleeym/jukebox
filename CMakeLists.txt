cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(jukebox VERSION 2.9.0)

file(GLOB SOURCES
    src/ui/*.cpp
    src/ui/list/*.cpp
    src/managers/*.cpp
    src/hooks/*.cpp
    src/events/*.cpp
    src/api/*.cpp
	src/*.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (PROJECT_IS_TOP_LEVEL)
  target_compile_definitions(${PROJECT_NAME} PRIVATE FLEYM_JUKEBOX_EXPORTING)
endif()

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} $ENV{GEODE_SDK}/build)

target_link_libraries(${PROJECT_NAME} geode-sdk)
create_geode_file(${PROJECT_NAME})
