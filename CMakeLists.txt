# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "~/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_person_detection_thermal C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico_person_detection_thermal main.c MLX90640_I2C_Driver.c MLX90640_API.c)

pico_set_program_name(pico_person_detection_thermal "pico_person_detection_thermal")
pico_set_program_version(pico_person_detection_thermal "0.1")

pico_enable_stdio_uart(pico_person_detection_thermal 0)
pico_enable_stdio_usb(pico_person_detection_thermal 1)

# Add the standard library to the build
target_link_libraries(pico_person_detection_thermal pico_stdlib)

# Add any user requested libraries
target_link_libraries(pico_person_detection_thermal
        hardware_i2c
        )

pico_add_extra_outputs(pico_person_detection_thermal)

