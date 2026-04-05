
cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)

add_executable(aes42hat)

generate_header(aes42hat cxx src4392 TI/SRC4392 .hpp)
generate_header(aes42hat cxx arm ARM/NVIC .hpp)
generate_header(aes42hat cxx lpc865 NXP/LPC8/LPC86x/LPC865 .hpp)
generate_header(aes42hat cxx lpc865 NXP/LPC8/LPC86x/LPC86x_clocks .hpp)

# Build sodaCat-generated register headers as C++20 modules.
# The hwreg support header is the foundation module; peripheral and chip
# headers import it.  LPC86x_clocks is excluded (clock tree generator
# does not yet support module mode).
target_compile_options(aes42hat PRIVATE -fmodules-ts)
build_system_header_units(aes42hat
    algorithm array bit cassert cstddef cstdint cstring
    initializer_list iterator span string_view
    type_traits utility variant version)
generate_support_module(aes42hat cxx hwreg.hpp)
foreach(_mod ACOMP ADC CRC SmartDMA FLASH_CTRL FTM GPIO I2C I3C
             INPUTMUX IOCON MRT NVIC PINT PMU SPI SWM SYSCON
             USART WKT WWDT LPC865 SRC4392)
    generate_module(aes42hat ${CMAKE_CURRENT_BINARY_DIR}/${_mod}.hpp)
endforeach()

target_include_directories(aes42hat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}     # for generated headers
    ${SODACAT_LOCAL_DIR}            # for headers that come with the generator
)

set_target_properties(aes42hat PROPERTIES
    LINK_DEPENDS "${CMAKE_SOURCE_DIR}/linker/lpc865.ld"
)

target_link_options(aes42hat PRIVATE
    -T${CMAKE_SOURCE_DIR}/linker/lpc865.ld
    -nostartfiles
    -Wl,--gc-sections
)

target_sources(aes42hat PUBLIC
    nvic_drv.cpp
    channel.cpp
    clkmgr.cpp
    dma_drv.cpp
    ftm_drv.cpp
    handler.cpp
    i2c_tgt_drv.cpp
    pint_drv.cpp
    spi_drv.cpp
    spi_queue.cpp
    src4392_drv.cpp 
    usart_drv.cpp
    wkt_drv.cpp
    startup.cpp
    main.cpp
)
