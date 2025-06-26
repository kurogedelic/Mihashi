# Mihashi Simple Dual USB Implementation
# Device + Simple Host Bridge
cmake_minimum_required(VERSION 3.12)

# Set board to RP2350
set(PICO_BOARD pico2)

# Include pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(mihashi_simple_dual)

# Initialize SDK
pico_sdk_init()

# Add executable
add_executable(mihashi_simple_dual
    src/main_simple_dual.c
    src/usb_descriptors.c
)

# Include directories
target_include_directories(mihashi_simple_dual PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link libraries
target_link_libraries(mihashi_simple_dual PRIVATE
    pico_stdlib
    pico_multicore
    tinyusb_device
    tinyusb_board
    hardware_gpio
    hardware_clocks
)

# Create map/bin/hex file
pico_add_extra_outputs(mihashi_simple_dual)

# UART output
pico_enable_stdio_usb(mihashi_simple_dual 0)
pico_enable_stdio_uart(mihashi_simple_dual 1)

# Additional compiler flags
target_compile_options(mihashi_simple_dual PRIVATE
    -Wall
    -Wextra
    -O2
)