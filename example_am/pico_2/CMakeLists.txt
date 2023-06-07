# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)
# set(PICO_SDK_PATH "/tree/downloaded_tools/pico/pico-sdk")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-pio-link-2 C CXX ASM)
set(PICO_COPY_TO_RAM 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(pico-pio-link-2
pico_2.c
../common/link_common.c
)

pico_generate_pio_header(pico-pio-link-2 ${CMAKE_CURRENT_LIST_DIR}/picoputer.pio)

pico_set_program_name(pico-pio-link-2 "pico-pio-link-2")
pico_set_program_version(pico-pio-link-2 "0.1")

pico_enable_stdio_usb(pico-pio-link-2 1)

# Add the standard library to the build
target_link_libraries(pico-pio-link-2 pico_stdlib)

# Add any user requested libraries
target_link_libraries(pico-pio-link-2
        hardware_pio
        hardware_clocks
	hardware_adc
        pico_multicore
        )

pico_add_extra_outputs(pico-pio-link-2)

