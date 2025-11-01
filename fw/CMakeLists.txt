
cmake_minimum_required(VERSION 3.28)

project(lpc865_blinky C CXX)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_C_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-unwind-tables")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-unwind-tables -fno-rtti -fno-exceptions")

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)
