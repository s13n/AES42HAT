/** @file
 * startup code for the AES42HAT
 */

#include "WKT.hpp"
#include "WWDT.hpp"
#include "LPC865.hpp"

extern "C" {
#   include "newlib_def.h"
#   include <stdint.h>
#   include <stdio.h>
}

using namespace lpc865;

/** @addtogroup AES42HAT
 * @{
 */

// The following initialization routine runs before the global data memory has been
// initialized. It puts the system in its desired running state. This includes
// pin configuration, clock configuration, and memory interface timing.
inline int sysinit() {
    // Use the FRO API ROM routine to set the FRO frequency
    struct PWRD {
        void (*set_fro_freq)(unsigned frequency);
    };
    struct ROM {
        const PWRD *pPWRD;
    };
    ROM **rom = (ROM **)0x0F001D98;
    (*rom)->pPWRD->set_fro_freq(60000);     // 60 MHz

    auto &syscon = *i_SYSCON.registers;     // SYSCON register set
    syscon.SYSAHBCLKCTRL0.set(0x31FFFEF7);
    syscon.SYSOSCCTRL.set(0x01);
    syscon.CLKOUTSEL.set(CLKOUTSEL_::MAIN_CLK);
    syscon.CLKOUTDIV.set(10);               // divide by 10
    syscon.PDRUNCFG.set(0xEDA0);
    syscon.LPOSCEN.set(3);                  // enable lp_osc for both WKT and WDT
    syscon.LPOSCEN.set(3);                  // enable lp_osc for both WKT and WDT
    syscon.WKTCLKSEL.set(1);                // select lp_osc
    syscon.FRODIRECTCLKUEN.set(0);
    auto frooscctrl = syscon.FROOSCCTRL.get();
    frooscctrl.FRO_DIRECT = 1;              // switch to 60 MHz
    syscon.FROOSCCTRL.set(frooscctrl);
    syscon.FRODIRECTCLKUEN.set(1);
/*
    auto &iocon = *i_IOCON.registers;       // IOCON register set
    iocon.PIO0_0.set(0x0000);   // MOSI
    iocon.PIO0_1.set(0x0000);   // ACLK
    iocon.PIO0_2.set(0x0000);   // SWDIO
    iocon.PIO0_3.set(0x0000);   // SWCLK
    iocon.PIO0_4.set(0x0000);   // CSB
    iocon.PIO0_5.set(0x0000);   // RESET
    iocon.PIO0_6.set(0x0000);   // PVB
    iocon.PIO0_7.set(0x0000);   // PVA
    iocon.PIO0_8.set(0x0000);   // MCLK
    iocon.PIO0_9.set(0x0000);   // TP9
    iocon.PIO0_10.set(0x0000);  // SCL
    iocon.PIO0_11.set(0x0000);  // SDA
    iocon.PIO0_12.set(0x0000);  // REQ/ISP
    iocon.PIO0_13.set(0x0000);  // SCLK
    iocon.PIO0_14.set(0x0000);  // PVC
    iocon.PIO0_15.set(0x0000);  // MODA
    iocon.PIO0_16.set(0x0000);  // MODB
    iocon.PIO0_17.set(0x0000);  // BLS
    iocon.PIO0_18.set(0x0000);  // MISO
    iocon.PIO0_19.set(0x0000);  // INTA
    iocon.PIO0_20.set(0x0000);  // INTB
    iocon.PIO0_21.set(0x0000);  // UCKB
    iocon.PIO0_22.set(0x0000);  // UCKA
    iocon.PIO0_23.set(0x0000);  // PVD
    iocon.PIO0_24.set(0x0000);  // H2P
    iocon.PIO0_25.set(0x0000);  // P2H
    iocon.PIO0_26.set(0x0000);  // UBTB
    iocon.PIO0_27.set(0x0000);  // UBTA
    iocon.PIO0_28.set(0x0000);  // CSA
    iocon.PIO0_29.set(0x0000);  // WCLK
    iocon.PIO0_30.set(0x0000);  // BCK
    iocon.PIO0_31.set(0x0000);  // MODC
    iocon.PIO1_0.set(0x0000);   // MODD
    iocon.PIO1_1.set(0x0000);   // CSD
    iocon.PIO1_2.set(0x0000);   // CSC
    iocon.PIO1_3.set(0x0000);   // UCKD
    iocon.PIO1_4.set(0x0000);   // UCKC
    iocon.PIO1_5.set(0x0000);   // INTC
    iocon.PIO1_6.set(0x0000);   // INTD
    iocon.PIO1_7.set(0x0000);   // LED
    iocon.PIO1_8.set(0x0000);   // UBTD
    iocon.PIO1_9.set(0x0000);   // UBTC
*/
    auto &swm0 = *i_SWM0.registers;         // SWM0 register set
    swm0.PINASSIGN0.set(0xFFFF1819);
    swm0.PINASSIGN1.set(0xFFFFFFFF);
    swm0.PINASSIGN2.set(0xFFFFFFFF);
    swm0.PINASSIGN3.set(0x0DFFFFFF);
    swm0.PINASSIGN4.set(0x041C1200);
    swm0.PINASSIGN5.set(0xFFFF2122);
    swm0.PINASSIGN6.set(0x0BFFFFFF);
    swm0.PINASSIGN7.set(0xFFFFFF0A);
    swm0.PINASSIGN8.set(0xFFFF09FF);        // CLKOUT on TP9
    swm0.FTM_PINASSIGN0.set(0xFFFFFFFF);
    swm0.PINENABLE0.set(0xFFFF081F);        // enable ADC0..3, CLKIN, XTALIN, RESET and SWD

    auto &inputmux = *i_INPUTMUX.registers; // INPUTMUX register set

    return 0;
}

extern "C" {

void _exit(int status) {
//    fprintf(stderr, "\nexiting...\n");
//    fflush(stderr);
    while(1);   // _exit doesn't return
}

void _init(void) {}
void _fini(void) {}

extern int main();

/** Reset entry point.
 * Sets up a simple runtime environment and initializes the C/C++ library.
 */
void ResetISR(void) {
    sysinit();              // early system initialization to make memory usable
    startup_meminit();      // initialize .data and .bss
    __libc_init_array();    // C++ library initialization
    _exit(main());          // _exit() never returns
}

/** The exception vector table.
 * This relies on the linker script to place at correct location in memory.
 */
__attribute__((used, section(".isr_vector")))
void (* const vector_table[])(void) = {
    &__stack,       // The initial stack pointer
    ResetISR,       // The reset handler
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,

    nullptr, // 0 040 SPI0_IRQ SPI0 interrupt See Table 282 “SPI Interrupt Enable read and Set register (INTENSET, addresses 0x4005 800C (SPI0), 0x4005 C00C (SPI1)) bit description”.
    nullptr, // 1 044 SPI1_IRQ SPI1 interrupt Same as SPI0_IRQ
    nullptr, // 2 048 - Reserved -
    nullptr, // 3 04C UART0_IRQ USART0 interrupt See Table 268 “USART Interrupt Enable read and set register (INTENSET, address 0x4006 400C (USART0), 0x4006 800C (USART1), 0x4006C00C (USART2)) bit description”
    nullptr, // 4 050 UART1_IRQ USART1 interrupt Same as UART0_IRQ
    nullptr, // 5 054 UART2_IRQ USART2 interrupt Same as UART0_IRQ
    nullptr, // 6 058 FTM0_IRQ FlexTimer0 interrupt -
    nullptr, // 7 05C FTM1_IRQ FlexTimer1 interrupt -
    nullptr, // 8 060 I2C0_IRQ I2C0 interrupt See Table 298 “Interrupt Enable Clear register (INTENCLR, address 0x4005 000C (I2C0)) bit description”.
    nullptr, // 9 064 - Reserved -
    nullptr, // 10 068 MRT_IRQ Multi-rate timer interrupt Global MRT interrupt. GFLAG0 GFLAG1 GFLAG2 GFLAG3
    nullptr, // 11 06C ACMP_IRQ Analog comparator interrupt COMPEDGE - rising, falling, or both edges can set the bit.
    nullptr, // 12 070 WDT_IRQ Windowed watchdog timer interrupt WARNINT - watchdog warning interrupt
    nullptr, // 13 074 BOD_IRQ BOD interrupt BODINTVAL - BOD interrupt level
    nullptr, // 14 078 FLASH_IRQ Flash interrupt -
    nullptr, // 15 07C WKT_IRQ Self-wake-up timer interrupt ALARMFLAG
    nullptr, // 16 080 ADC_SEQA_IRQ ADC sequence A completion interrupt -
    nullptr, // 17 084 ADC_SEQB_IRQ ADC sequence B completion interrupt -
    nullptr, // 18 088 ADC_THCMP_IRQ ADC threshold compare interrupt -
    nullptr, // 19 08C ADC_OVR_IRQ ADC overrun interrupt -
    nullptr, // 20 090 DMA_IRQ DMA0 controller interrupt -
    nullptr, // 21 094 I3C0_IRQ I3C interface 0 interrupt -
    nullptr, // 22 098 GPIO_HS_IRQ0 GPIO group A interrupt -.
    nullptr, // 23 09C GPIO_HS_IRQ1 GPIO group B interrupt -
    nullptr, // 24 0A0 PININT0_IRQ Pin interrupt 0 or pattern match engine slice 0 interrupt PSTAT - pin interrupt status
    nullptr, // 25 0A4 PININT1_IRQ Pin interrupt 1 or pattern match engine slice 1 interrupt PSTAT - pin interrupt status
    nullptr, // 26 0A8 PININT2_IRQ Pin interrupt 2 or pattern match engine slice 2 interrupt PSTAT - pin interrupt status
    nullptr, // 27 0AC PININT3_IRQ Pin interrupt 3 or pattern match engine slice 3 interrupt PSTAT - pin interrupt status
    nullptr, // 28 0B0 PININT4_IRQ Pin interrupt 4 or pattern match engine slice 4 interrupt PSTAT - pin interrupt status
    nullptr, // 29 0B4 PININT5_IRQ Pin interrupt 5 or pattern match engine slice 5 interrupt PSTAT - pin interrupt status
    nullptr, // 30 0B8 PININT6_IRQ Pin interrupt 6 or pattern match engine slice 6 interrupt PSTAT - pin interrupt status
    nullptr, // 31 0BC PININT7_IRQ Pin interrupt 7 or pattern match engine slice 7 interrupt PSTAT - pin interrupt status
};

/** Number of entries in exception vector table.
 *
 */
unsigned const short nvic_num_exceptions = sizeof(vector_table) / sizeof(vector_table[0]);

/** Number of priority bits implemented by NVIC.
 * We're running on the M0+, which has 2 bits.
 */
unsigned const short nvic_prio_bits = 2;

} // extern "C"

void *__dso_handle = nullptr;

/** @} */
