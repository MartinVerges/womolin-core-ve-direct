cmake_minimum_required(VERSION 3.14)
project(ve2mqtt)

include(ExternalProject)
include(FetchContent)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(PAHO_ENABLE_TESTING false)
set(PAHO_BUILD_STATIC false)
set(PAHO_BUILD_SHARED true)
set(PAHO_WITH_SSL false)
set(PAHO_BUILD_DOCUMENTATION false)
set(PAHO_BUILD_SAMPLES false)
set(PAHO_BUILD_DEB_PACKAGE false)
set(PAHO_ENABLE_CPACK true)
set(PAHO_HIGH_PERFORMANCE true)

FetchContent_Declare(mqtt                  GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git GIT_TAG v1.3.10)
FetchContent_Declare(VeDirectFrameHandler  GIT_REPOSITORY https://gitlab.womolin.de/martin.verges/VeDirectFrameHandler.git)
FetchContent_MakeAvailable(mqtt VeDirectFrameHandler)

include_directories(${vedirectframehandler_SOURCE_DIR})
include_directories(${mqtt_SOURCE_DIR}/src/)

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_executable(ve2mqtt main.cpp)
target_link_libraries(ve2mqtt VeDirectFrameHandler paho-mqtt3c)

#install(TARGETS paho-mqtt3c-static paho-mqtt3a-static EXCLUDE_FROM_ALL)

install(FILES ${CMAKE_BINARY_DIR}/ve2mqtt PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ DESTINATION /usr/local/bin/womolin/)
install(FILES ${CMAKE_SOURCE_DIR}/systemd/ve2mqtt@.service DESTINATION /etc/systemd/system/)
install(FILES ${CMAKE_SOURCE_DIR}/systemd/environment DESTINATION /etc/default/ RENAME womolin-ve2mqtt-env)

install(CODE "EXECUTE_PROCESS(COMMAND systemctl daemon-reload)")
