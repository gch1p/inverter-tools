# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wno-psabi)

project(inverter-tools VERSION 1.4.99)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr/local/bin)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


include(GNUInstallDirs)


# find hidapi
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_library(HIDAPI_LIBRARY hidapi-hidraw)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    find_library(HIDAPI_LIBRARY hidapi)
endif()
find_path(HIDAPI_INCLUDE_DIR hidapi/hidapi.h)


# find libserialport
find_library(LIBSERIALPORT_LIBRARY serialport)
find_path(LIBSERIALPORT_INCLUDE_DIR libserialport.h)


add_executable(inverterctl
        src/inverterctl.cc
        src/p18/defines.cc
        src/p18/client.cc
        src/p18/functions.cc
        src/p18/response.cc
        src/util.cc
        src/p18/commands.cc
        src/common.cc
        src/formatter/formatter.cc
        src/voltronic/crc.cc
        src/voltronic/usb_device.cc
        src/voltronic/device.cc
        src/voltronic/time.cc
        src/voltronic/serial_device.cc
        src/voltronic/pseudo_device.cc)
target_include_directories(inverterctl PRIVATE .)
target_link_libraries(inverterctl m ${HIDAPI_LIBRARY} ${LIBSERIALPORT_LIBRARY})
target_compile_definitions(inverterctl PUBLIC INVERTERCTL)
target_include_directories(inverterctl PRIVATE
        ${HIDAPI_INCLUDE_DIR}
        ${LIBSERIALPORT_INCLUDE_DIR}
        third_party
        third_party/json/single_include)
install(TARGETS inverterctl
        RUNTIME DESTINATION bin)


add_executable(inverterd
        src/inverterd.cc
        src/common.cc
        src/util.cc
        src/server/server.cc
        src/server/connection.cc
        src/server/signal.cc
        src/p18/commands.cc
        src/p18/defines.cc
        src/p18/client.cc
        src/p18/functions.cc
        src/p18/response.cc
        src/formatter/formatter.cc
        src/voltronic/crc.cc
        src/voltronic/usb_device.cc
        src/voltronic/device.cc
        src/voltronic/time.cc
        src/voltronic/serial_device.cc
        src/voltronic/pseudo_device.cc)
target_include_directories(inverterd PRIVATE .)
target_compile_definitions(inverterd PUBLIC INVERTERD)
target_link_libraries(inverterd
        m pthread
        ${HIDAPI_LIBRARY}
        ${LIBSERIALPORT_LIBRARY})
target_include_directories(inverterd PRIVATE
        ${HIDAPI_INCLUDE_DIR}
        ${LIBSERIALPORT_INCLUDE_DIR}
        third_party
        third_party/json/single_include)
install(TARGETS inverterd
        RUNTIME DESTINATION bin)


add_executable(testserial src/testserial.cc)
target_include_directories(testserial PRIVATE .)
target_link_libraries(testserial ${LIBSERIALPORT_LIBRARY})
target_include_directories(testserial PRIVATE
        ${LIBSERIALPORT_INCLUDE_DIR}
        third_party/hexdump)