# arm-none-eabi.toolchain.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_C_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-unwind-tables")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-exceptions -fno-unwind-tables -fno-rtti")

set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Wl,--gc-sections -Wl,--no-warn-rwx-segments")