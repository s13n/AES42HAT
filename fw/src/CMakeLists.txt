
cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)

add_executable(aes42hat)

generate_header(aes42hat generate_peripheral_header src4392 SRC4392 .hpp)

generate_header(aes42hat generate_peripheral_header lpc865 ACOMP .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 ADC .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 CRC .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 SmartDMA .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 FLASH_CTRL .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 FTM .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 GPIO .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 I2C .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 I3C .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 INPUTMUX .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 IOCON .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 MRT .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 NVIC .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 PINT .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 PMU .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 SPI .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 SWM .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 SYSCON .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 USART .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 WKT .hpp)
generate_header(aes42hat generate_peripheral_header lpc865 WWDT .hpp)

generate_header(aes42hat generate_clocktree_header lpc865 LPC865_clocks .hpp)

generate_header(aes42hat generate_chip_header lpc865 LPC865 .hpp)

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
    src4392_drv.cpp 
    usart_drv.cpp
    wkt_drv.cpp
    startup.cpp
    main.cpp
)
