cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

include(config/pico_sdk_import.cmake)

project(svi-328-picorom C CXX ASM)

pico_sdk_init()

add_executable(svi-328-picorom
    c/main.c
    c/svi-328-cartridge.c
    c/log.c
    c/wifi.c
    )

target_include_directories(svi-328-picorom PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/c # lwipopts.h
    )

target_link_libraries(svi-328-picorom
    pico_stdlib
    pico_multicore
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip
    pico_lwip_http
)

set_target_properties(svi-328-picorom PROPERTIES PICO_TARGET_LINKER_SCRIPT 
    ${CMAKE_CURRENT_LIST_DIR}/config/memmap_rp2350.ld)

pico_enable_stdio_usb(svi-328-picorom 1)
pico_enable_stdio_uart(svi-328-picorom 0)

pico_add_extra_outputs(svi-328-picorom)

