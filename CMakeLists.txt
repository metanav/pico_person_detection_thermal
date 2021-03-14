# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "~/pico/pico-sdk")
set(MODEL_FOLDER .)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_person_detection_thermal C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(pico_person_detection_thermal 
               main.cpp 
               ei_classifier_porting.cpp 
               MLX90640_I2C_Driver.cpp 
               MLX90640_API.cpp
               ILI9341.cpp)

pico_set_program_name(pico_person_detection_thermal "pico_person_detection_thermal")
pico_set_program_version(pico_person_detection_thermal "0.1")

pico_enable_stdio_uart(pico_person_detection_thermal 0)
pico_enable_stdio_usb(pico_person_detection_thermal 1)

include(${MODEL_FOLDER}/edge-impulse-sdk/cmake/utils.cmake)
target_include_directories(pico_person_detection_thermal PRIVATE
    ${MODEL_FOLDER}
    ${MODEL_FOLDER}/tflite-model
    ${MODEL_FOLDER}/model-parameters
    ${MODEL_FOLDER}/edge-impulse-sdk/classifier
    ${MODEL_FOLDER}/edge-impulse-sdk/porting
)

# find edge impulse sdk and model source files
RECURSIVE_FIND_FILE(SOURCE_FILES "${MODEL_FOLDER}/edge-impulse-sdk" "*.cpp")
RECURSIVE_FIND_FILE(MODEL_FILES "${MODEL_FOLDER}/tflite-model" "*.cpp")
RECURSIVE_FIND_FILE(CC_FILES "${MODEL_FOLDER}/edge-impulse-sdk/tensorflow" "*.cc")
RECURSIVE_FIND_FILE(C_FILES "${MODEL_FOLDER}/edge-impulse-sdk/tensorflow" "*.c")
list(APPEND SOURCE_FILES ${CC_FILES})
list(APPEND SOURCE_FILES ${C_FILES})
list(APPEND SOURCE_FILES ${MODEL_FILES})

# add all sources to the project
target_sources(pico_person_detection_thermal PRIVATE ${SOURCE_FILES})

# Add the standard library to the build
target_link_libraries(pico_person_detection_thermal pico_stdlib)

# Add any user requested libraries
target_link_libraries(pico_person_detection_thermal
        hardware_i2c
        hardware_spi)

pico_add_extra_outputs(pico_person_detection_thermal)

