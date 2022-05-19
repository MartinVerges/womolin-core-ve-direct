cmake_minimum_required(VERSION 3.14)
project(ve2mqtt)

include(ExternalProject)
include(FetchContent)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FetchContent_Declare(mqtt                  GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git GIT_TAG v1.3.10)
FetchContent_Declare(VeDirectFrameHandler  GIT_REPOSITORY https://gitlab.womolin.de/martin.verges/VeDirectFrameHandler.git)
FetchContent_MakeAvailable(mqtt VeDirectFrameHandler)

include_directories(${vedirectframehandler_SOURCE_DIR})
include_directories(${mqtt_SOURCE_DIR}/src/)

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_executable(ve2mqtt main.cpp)
target_link_libraries(ve2mqtt VeDirectFrameHandler paho-mqtt3c)
