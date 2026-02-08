cmake_minimum_required(VERSION 3.22)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(jemgui_test_jstm C CXX ASM)

include(FetchContent)

FetchContent_Declare(
    jstm
    GIT_REPOSITORY https://github.com/ImArjunJ/jstm.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(jstm)

set(JSTM_LINKER_SCRIPT "${jstm_SOURCE_DIR}/STM32F746ZGTx_FLASH.ld"
    CACHE FILEPATH "")

add_link_options(-T${JSTM_LINKER_SCRIPT} -Wl,-Map=output.map -Wl,--gc-sections)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/jemgui)

add_executable(jemgui_test_jstm main.cpp)

target_link_libraries(jemgui_test_jstm PRIVATE
    jemgui
    jstm_hal
    jstm_ili9341
    jstm_xpt2046
)

set_target_properties(jemgui_test_jstm PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS "${JSTM_LINKER_SCRIPT}"
)

add_custom_command(TARGET jemgui_test_jstm POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:jemgui_test_jstm>
)

add_custom_command(TARGET jemgui_test_jstm POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:jemgui_test_jstm>
            ${CMAKE_CURRENT_BINARY_DIR}/jemgui_test_jstm.bin
)
