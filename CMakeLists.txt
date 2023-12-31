cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(cmake/pico_sdk_import.cmake)

project(pico_sd_ctrl)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(include/no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)

add_executable(${PROJECT_NAME} 
    src/main.c
)

# Add the standard library and FatFS/SPI to the build
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_pwm
    FatFs_SPI
)

add_subdirectory(src)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})
