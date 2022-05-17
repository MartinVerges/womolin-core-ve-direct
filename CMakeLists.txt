cmake_minimum_required(VERSION 3.14)
project(ve2mqtt)

add_compile_options(-Wall -Wextra -pedantic -Werror)

include(ExternalProject)
include(FetchContent)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FetchContent_Declare(VeDirectFrameHandler  GIT_REPOSITORY https://gitlab.womolin.de/martin.verges/VeDirectFrameHandler.git)
FetchContent_MakeAvailable(VeDirectFrameHandler)

include_directories(${vedirectframehandler_SOURCE_DIR})
add_executable(ve2mqtt main.cpp)
target_link_libraries(ve2mqtt VeDirectFrameHandler)


# Trash
#FetchContent_Declare(ArduinoJson           GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git               GIT_TAG v6.19.4)
#FetchContent_MakeAvailable(ArduinoJson)

#FetchContent_Declare(ArduinoESP32          GIT_REPOSITORY https://github.com/espressif/arduino-esp32.git             GIT_TAG 2.0.3)
#FetchContent_Declare(ESP_IDF               GIT_REPOSITORY https://github.com/espressif/esp-idf.git                   GIT_TAG v4.4.1)
#FetchContent_Populate(ArduinoESP32)
#FetchContent_MakeAvailable(ArduinoESP32)
#FetchContent_MakeAvailable(ESP_IDF)

#file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${arduinoesp32_SOURCE_DIR}/**/.h ${arduinoesp32_SOURCE_DIR}/**/.c ${arduinoesp32_SOURCE_DIR}/**/.cpp)
##include_regular_expression(${arduinoesp32_SOURCE_DIR}/.*)
#add_library(esp32 ${sources})

#include_directories(${esp_idf_SOURCE_DIR})
#add_subdirectory(${vedirectframehandler_SOURCE_DIR} ${vedirectframehandler_BINARY_DIR})