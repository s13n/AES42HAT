
cmake_minimum_required(VERSION 3.28)

add_executable(aes42hat)

macro(generate_peripheral model)
    add_custom_command(OUTPUT ${model}.cpp ${model}.hpp
        COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/generate_peripheral_header.py ${CMAKE_SOURCE_DIR}/model/${model}.yaml ${CMAKE_CURRENT_BINARY_DIR}/${model} lpc865
        MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/model/${model}.yaml
        DEPENDS ${CMAKE_SOURCE_DIR}/tools/generate_peripheral_header.py
        COMMENT "Generating ${model}.hpp for LPC865"
    )
    target_sources(aes42hat PUBLIC
        ${model}.hpp
    )
endmacro()

generate_peripheral(ACOMP)
generate_peripheral(ADC)
generate_peripheral(CRC)
generate_peripheral(DMA)
generate_peripheral(FLASH_CTRL)
generate_peripheral(FTM0)
generate_peripheral(FTM1)
generate_peripheral(GPIO)
generate_peripheral(I2C)
generate_peripheral(I3C)
generate_peripheral(INPUTMUX)
generate_peripheral(IOCON)
generate_peripheral(MRT)
generate_peripheral(PINT)
generate_peripheral(PMU)
generate_peripheral(SPI)
generate_peripheral(SWM)
generate_peripheral(SYSCON)
generate_peripheral(USART)
generate_peripheral(WKT)
generate_peripheral(WWDT)

add_custom_command(OUTPUT LPC865.cpp LPC865.hpp
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/generate_chip_header.py ${CMAKE_SOURCE_DIR}/model/LPC865.yaml ${CMAKE_CURRENT_BINARY_DIR}/LPC865 lpc865
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/model/LPC865.yaml
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/generate_chip_header.py
    COMMENT "Generating LPC865.hpp for LPC865"
)

target_include_directories(aes42hat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_sources(aes42hat PUBLIC
    LPC865.hpp
    startup.cpp
    main.cpp
    dma_drv.cpp
    spi_drv.cpp
)

target_link_options(aes42hat PRIVATE
    -T${CMAKE_SOURCE_DIR}/linker/lpc865.ld
    -nostartfiles
    -Wl,--gc-sections
)
