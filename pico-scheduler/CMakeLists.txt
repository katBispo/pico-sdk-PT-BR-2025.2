cmake_minimum_required(VERSION 3.13)

set(PICO_NO_PICOTOOL 1)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_escalonador C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_escalonador
    app/main.c
    core/scheduler.c
    hal/console.c
)

target_include_directories(pico_escalonador PRIVATE
    app
    core
    hal
)

target_link_libraries(pico_escalonador pico_stdlib)

pico_add_extra_outputs(pico_escalonador)