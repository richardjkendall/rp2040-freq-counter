# What CMake to start at
cmake_minimum_required(VERSION 3.12)
 
# Include the subsidiary .cmake file to get the SDK
include(pico_sdk_import.cmake)
 
# Set the name and version of the project
project(FreqCounter VERSION 1.0.0)

# Initalise the SDK
pico_sdk_init()

# Link the Project to a source file (step 4.6)
add_executable(FreqCounter ${CMAKE_CURRENT_LIST_DIR}/main.c)

# generate C header for the PIO program
pico_generate_pio_header(FreqCounter ${CMAKE_CURRENT_LIST_DIR}/counter.pio)

# Link the Project to an extra library (pico_stdlib)
target_link_libraries(FreqCounter pico_stdlib hardware_pwm hardware_gpio hardware_irq hardware_pio hardware_dma)
 
# Enable USB, UART output
pico_enable_stdio_usb(FreqCounter 1)
pico_enable_stdio_uart(FreqCounter 1)
 
# Enable extra outputs (SWD?)
pico_add_extra_outputs(FreqCounter)