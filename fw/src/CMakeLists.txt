
cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)

add_executable(aes42hat)

macro(generate_peripheral namespace model)
    add_custom_command(OUTPUT ${model}.hpp
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/generate_peripheral_header.py ${CMAKE_SOURCE_DIR}/model/${model}.yaml ${CMAKE_CURRENT_BINARY_DIR}/${model} ${namespace}
        MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/model/${model}.yaml
        DEPENDS ${CMAKE_SOURCE_DIR}/tools/generate_peripheral_header.py
        COMMENT "Generating ${model}.hpp in namespace ${namespace}"
    )
    target_sources(aes42hat PUBLIC
        ${model}.hpp
    )
endmacro()

generate_peripheral(lpc865 ACOMP)
generate_peripheral(lpc865 ADC)
generate_peripheral(lpc865 CRC)
generate_peripheral(lpc865 DMA)
generate_peripheral(lpc865 FLASH_CTRL)
generate_peripheral(lpc865 FTM)
generate_peripheral(lpc865 GPIO)
generate_peripheral(lpc865 I2C)
generate_peripheral(lpc865 I3C)
generate_peripheral(lpc865 INPUTMUX)
generate_peripheral(lpc865 IOCON)
generate_peripheral(lpc865 MRT)
generate_peripheral(lpc865 NVIC)
generate_peripheral(lpc865 PINT)
generate_peripheral(lpc865 PMU)
generate_peripheral(lpc865 SPI)
generate_peripheral(lpc865 SWM)
generate_peripheral(lpc865 SYSCON)
generate_peripheral(lpc865 USART)
generate_peripheral(lpc865 WKT)
generate_peripheral(lpc865 WWDT)
generate_peripheral(src4392 SRC4392)

add_custom_command(OUTPUT clocks.hpp
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/generate_clocktree_header.py ${CMAKE_SOURCE_DIR}/model/LPC865_clocks.yaml ${CMAKE_CURRENT_BINARY_DIR}/clocks lpc865
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/model/LPC865_clocks.yaml
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/generate_clocktree_header.py
    COMMENT "Generating clocks.hpp in namespace lpc865"
)

add_custom_command(OUTPUT chip.hpp
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/generate_chip_header.py ${CMAKE_SOURCE_DIR}/model/LPC865.yaml ${CMAKE_CURRENT_BINARY_DIR}/chip lpc865
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/model/LPC865.yaml
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/generate_chip_header.py
    COMMENT "Generating chip.hpp in namespace lpc865"
)

target_include_directories(aes42hat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}     # for generated headers
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
    chip.hpp
    clocks.hpp
    channel.cpp
    dma_drv.cpp
    ftm_drv.cpp
    handler.cpp
    i2c_tgt_drv.cpp
    nvic_drv.cpp
    spi_drv.cpp
    src4392_drv.cpp 
    usart_drv.cpp
    startup.cpp
    main.cpp
)
