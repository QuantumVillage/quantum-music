cmake_minimum_required(VERSION 3.13)

set(BOARD pico_sdk)
include(pico_sdk_import.cmake)

set(target_proj qmidi)
project(${target_proj} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/qsim)


add_executable(${target_proj}
    qmidi.c
)
#
#if(DEFINED ENV{MIDI_UART_NUM})
#    set(MIDI_UART_NUM $ENV{MIDI_UART_NUM})
#endif()
#if(DEFINED ENV{MIDI_UART_TX_GPIO})
#    set(MIDI_UART_TX_GPIO $ENV{MIDI_UART_TX_GPIO})
#endif()
#if(DEFINED ENV{MIDI_UART_RX_GPIO})
#    set(MIDI_UART_RX_GPIO $ENV{MIDI_UART_RX_GPIO})
#endif()

set(MIDI_UART_NUM 1)
set(MIDI_UART_TX_GPIO 4)
set(MIDI_UART_RX_GPIO 5)

if (DEFINED MIDI_UART_NUM)
    if (${MIDI_UART_NUM} EQUAL 1)
        # MIDI is UART 1, so can enable UART 0 debugging
        pico_enable_stdio_uart(${target_proj} 1)
        if(DEFINED MIDI_UART_TX_GPIO AND DEFINED MIDI_UART_RX_GPIO)
            target_compile_options(${target_proj} PRIVATE -DMIDI_UART_NUM=${MIDI_UART_NUM} -DMIDI_UART_TX_GPIO=${MIDI_UART_TX_GPIO} -DMIDI_UART_RX_GPIO=${MIDI_UART_RX_GPIO})
        endif()
    elseif(NOT ${MIDI_UART_NUM} EQUAL 0)
        message(FATAL_ERROR "Legal values for MIDI_UART_NUM are 0 or 1, CMake will exit." )
    elseif(NOT DEFINED MIDI_UART_TX_GPIO OR NOT DEFINED MIDI_UART_RX_GPIO)
        message(FATAL_ERROR "You must define MIDI_UART_TX_GPIO and MIDI_UART_RX_GPIO if you define MIDI_UART_NUM 0, CMake will exit." )
    else()
        message("UART debugging is disabled if you use UART 0 for MIDI")
        pico_enable_stdio_uart(${target_proj} 0)
        target_compile_options(${target_proj} PRIVATE -DMIDI_UART_NUM=${MIDI_UART_NUM} -DMIDI_UART_TX_GPIO=${MIDI_UART_TX_GPIO} -DMIDI_UART_RX_GPIO=${MIDI_UART_RX_GPIO})
    endif()
else()
    # not defined, so will be UART 1. Can enable UART 0 debugging
    pico_enable_stdio_uart(${target_proj} 1)
endif()
target_include_directories(${target_proj} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_options(${target_proj} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_proj} PRIVATE -Wall -Wextra)
target_link_libraries(${target_proj} qsim pico_stdlib)

pico_add_extra_outputs(${target_proj})
