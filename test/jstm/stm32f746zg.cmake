set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

find_program(CMAKE_C_COMPILER   arm-none-eabi-gcc)
find_program(CMAKE_CXX_COMPILER arm-none-eabi-g++)
find_program(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
find_program(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
find_program(CMAKE_OBJDUMP      arm-none-eabi-objdump)
find_program(CMAKE_SIZE         arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(MCU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT          "${MCU_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT        "${MCU_FLAGS}")
set(CMAKE_ASM_FLAGS_INIT        "${MCU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${MCU_FLAGS} -specs=nano.specs -Wl,--gc-sections")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
