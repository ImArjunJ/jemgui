cmake_minimum_required(VERSION 3.13)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion RISCV_ZCB_RPI_2_2_0_3)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if(EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico2_w CACHE STRING "Board type")

include(${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake)

project(jemgui_test_jpico C CXX ASM)

pico_sdk_init()

include(FetchContent)

FetchContent_Declare(
    jpico
    GIT_REPOSITORY https://github.com/ImArjunJ/jpico.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(jpico)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/jemgui)

add_executable(jemgui_test_jpico main.cpp)

target_link_libraries(jemgui_test_jpico PRIVATE
    jemgui
    jpico_core
    jpico_hal
    jpico_ili9341
    jpico_xpt2046
    pico_stdlib
    hardware_spi
)

pico_enable_stdio_usb(jemgui_test_jpico 1)
pico_enable_stdio_uart(jemgui_test_jpico 0)
pico_add_extra_outputs(jemgui_test_jpico)
