
cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)

add_executable(aes42hat)

generate_header(aes42hat generate_peripheral_header src4392 SRC4392)

generate_header(aes42hat generate_peripheral_header lpc865 ACOMP)
generate_header(aes42hat generate_peripheral_header lpc865 ADC)
generate_header(aes42hat generate_peripheral_header lpc865 CRC)
generate_header(aes42hat generate_peripheral_header lpc865 DMA)
generate_header(aes42hat generate_peripheral_header lpc865 FLASH_CTRL)
generate_header(aes42hat generate_peripheral_header lpc865 FTM)
generate_header(aes42hat generate_peripheral_header lpc865 GPIO)
generate_header(aes42hat generate_peripheral_header lpc865 I2C)
generate_header(aes42hat generate_peripheral_header lpc865 I3C)
generate_header(aes42hat generate_peripheral_header lpc865 INPUTMUX)
generate_header(aes42hat generate_peripheral_header lpc865 IOCON)
generate_header(aes42hat generate_peripheral_header lpc865 MRT)
generate_header(aes42hat generate_peripheral_header lpc865 NVIC)
generate_header(aes42hat generate_peripheral_header lpc865 PINT)
generate_header(aes42hat generate_peripheral_header lpc865 PMU)
generate_header(aes42hat generate_peripheral_header lpc865 SPI)
generate_header(aes42hat generate_peripheral_header lpc865 SWM)
generate_header(aes42hat generate_peripheral_header lpc865 SYSCON)
generate_header(aes42hat generate_peripheral_header lpc865 USART)
generate_header(aes42hat generate_peripheral_header lpc865 WKT)
generate_header(aes42hat generate_peripheral_header lpc865 WWDT)

generate_header(aes42hat generate_clocktree_header lpc865 LPC865_clocks)

generate_header(aes42hat generate_chip_header lpc865 LPC865)

target_include_directories(aes42hat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}     # for generated headers
    ${CMAKE_SOURCE_DIR}/models      # for headers that come with the generator
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
    LPC865.hpp
    LPC865_clocks.hpp
    channel.cpp
    clkmgr.cpp
    dma_drv.cpp
    ftm_drv.cpp
    handler.cpp
    i2c_tgt_drv.cpp
    pint_drv.cpp
    spi_drv.cpp
    src4392_drv.cpp 
    usart_drv.cpp
    wkt_drv.cpp
    startup.cpp
    main.cpp
)
