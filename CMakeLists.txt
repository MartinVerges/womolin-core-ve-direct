cmake_minimum_required(VERSION 3.14)
project(ve2mqtt)

include(ExternalProject)
include(FetchContent)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_CPPLINT true)

set(VEDIRECT_BUILD_STATIC true CACHE BOOL "ignore")
set(PAHO_ENABLE_TESTING false CACHE BOOL "ignore")
set(PAHO_BUILD_STATIC true CACHE BOOL "ignore")
set(PAHO_BUILD_SHARED false CACHE BOOL "ignore")

FetchContent_Declare(mqtt                  GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git GIT_TAG v1.3.10)
FetchContent_Declare(VeDirectFrameHandler  GIT_REPOSITORY https://gitlab.womolin.de/martin.verges/VeDirectFrameHandler.git)
FetchContent_MakeAvailable(mqtt VeDirectFrameHandler)

include_directories(${vedirectframehandler_SOURCE_DIR})
include_directories(${mqtt_SOURCE_DIR}/src/)
include_directories(${CMAKE_SOURCE_DIR}/src/)

add_compile_options(-Wall -Wextra -pedantic -Werror -static)
add_executable(ve2mqtt src/ve2mqtt.cpp)
target_link_libraries(ve2mqtt VeDirectFrameHandler paho-mqtt3c-static)

install(FILES ${CMAKE_BINARY_DIR}/ve2mqtt PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ DESTINATION /usr/local/bin/womolin/)
install(FILES ${CMAKE_SOURCE_DIR}/systemd/ve2mqtt@.service DESTINATION /etc/systemd/system/)

if(NOT EXISTS "/etc/default/womolin-ve2mqtt")
    install(FILES ${CMAKE_SOURCE_DIR}/systemd/environment DESTINATION /etc/default/ RENAME womolin-ve2mqtt)
endif()

install(CODE "EXECUTE_PROCESS(COMMAND systemctl daemon-reload)")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Martin Verges <martin@verges.cc>")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.30)")

INCLUDE(CPack)

