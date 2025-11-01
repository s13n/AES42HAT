
cmake_minimum_required(VERSION 3.13)
project(lpc865_blinky C CXX)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_C_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-unwind-tables")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-unwind-tables -fno-rtti -fno-exceptions")

add_executable(blinky
    src/main.cpp
    src/startup.cpp
)

target_include_directories(blinky PRIVATE include)

target_link_options(blinky PRIVATE
    -T${CMAKE_SOURCE_DIR}/linker/lpc865.ld
    -nostartfiles
    -Wl,--gc-sections
)
