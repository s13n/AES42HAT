/** @file
 * startup code for the AES42HAT
 */

#include "WKT.hpp"
#include "WWDT.hpp"
#include "LPC865_clocks.hpp"

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
    // set up the clocking structure
    auto &syscon = *i_SYSCON.registers;     // SYSCON register set
    syscon.SYSOSCCTRL = SYSOSCCTRL{ .BYPASS=1, .FREQRANGE=0 };
    syscon.EXTCLKSEL = EXTCLKSEL{ .SEL=EXTCLKSEL_::CLK_IN };
    syscon.MAINCLKUEN = MAINCLKUEN{ .ENA=MAINCLKUEN_::NO_CHANGE };
    syscon.MAINCLKSEL = MAINCLKSEL{ .SEL=MAINCLKSEL_::FRO };
    syscon.MAINCLKUEN = MAINCLKUEN{ .ENA=MAINCLKUEN_::UPDATED };
    syscon.MAINCLKPLLUEN = MAINCLKPLLUEN{ .ENA=MAINCLKPLLUEN_::NO_CHANGE };
    syscon.MAINCLKPLLSEL = MAINCLKPLLSEL{ .SEL=MAINCLKPLLSEL_::MAIN_CLK_PRE_PLL };
    syscon.MAINCLKPLLUEN = MAINCLKPLLUEN{ .ENA=MAINCLKPLLUEN_::UPDATED };
    syscon.SYSAHBCLKDIV = SYSAHBCLKDIV{ .DIV=1 };
    syscon.SYSPLLCLKUEN = SYSPLLCLKUEN{ .ENA=SYSPLLCLKUEN_::NO_CHANGE };
    syscon.SYSPLLCLKSEL = SYSPLLCLKSEL{ .SEL=SYSPLLCLKSEL_::EXT_CLK };
    syscon.SYSPLLCLKUEN = SYSPLLCLKUEN{ .ENA=SYSPLLCLKUEN_::UPDATED };
    syscon.PDRUNCFG.set(0x8DA0);    // power down PLL
    syscon.SYSPLLCTRL = SYSPLLCTRL{ .MSEL=2, .PSEL=PSEL_3 };    // 10 MHz --> 240 MHz --> 30 MHz
    syscon.SYSPLLDIV = SYSPLLDIV{ .DIV=1 };
    syscon.PDRUNCFG.set(0x8D20);    // power up PLL
    i_PMU.registers->DPDCTRL = DPDCTRL{ .ULPOSCEN = 1 };
    syscon.LPOSCEN = LPOSCEN{ .WDT_CLK_EN=WDT_CLK_EN_::ENABLE, .WKT_CLK_EN=WKT_CLK_EN::ENABLE };
    syscon.WKTCLKSEL = WKTCLKSEL{ .SEL=WKTCLKSEL_::SYS_PLL };
    syscon.FRODIRECTCLKUEN = FRODIRECTCLKUEN{ .ENA=FRODIRECTCLKUEN_::NO_CHANGE };
    syscon.FROOSCCTRL = FROOSCCTRL{ .FRO_DIRECT=FRO_DIRECT_::ENABLED };
    syscon.FRODIRECTCLKUEN = FRODIRECTCLKUEN{ .ENA=FRODIRECTCLKUEN_::UPDATED };
    syscon.SYSAHBCLKCTRL0.set(0x31FFFEF7);
    syscon.FRG[0].FRGCLKSEL = FRGCLKSEL{ .SEL=FRGCLKSEL_::FRO };
    syscon.CLKOUTSEL = CLKOUTSEL{ .SEL=CLKOUTSEL_::SYS_PLL };
    syscon.CLKOUTDIV = CLKOUTDIV{ .DIV=10 };    // divide by 10
    syscon.FCLKSEL[0] = FCLKSEL{ .SEL=FCLKSEL_::FRO_DIV };  // USART0 clock == 30 MHz
    syscon.FCLKSEL[5] = FCLKSEL{ .SEL=FCLKSEL_::FRO };      // I2C0 clock == 60 MHz
    syscon.FCLKSEL2[0] = FCLKSEL2{ .SEL=FCLKSEL2_::FRO };   // SPI0 clock == 60 MHz
    syscon.FCLKSEL2[1] = FCLKSEL2{ .SEL=FCLKSEL2_::FRO };   // SPI1 clock == 60 MHz
    syscon.PINTSEL[0] = PINTSEL{ .INTPIN = 19 };    // INTA
    syscon.PINTSEL[1] = PINTSEL{ .INTPIN = 20 };    // INTB
    syscon.PINTSEL[2] = PINTSEL{ .INTPIN = 37 };    // INTC
    syscon.PINTSEL[3] = PINTSEL{ .INTPIN = 38 };    // INTD
    syscon.PINTSEL[4] = PINTSEL{ .INTPIN = 17 };    // BLS

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
    swm0.PINASSIGN0.set(0xFFFF1819);        // USART0
    swm0.PINASSIGN1.set(0xFFFFFFFF);        // USART0/1
    swm0.PINASSIGN2.set(0xFFFFFFFF);        // USART1/2
    swm0.PINASSIGN3.set(0x0DFFFFFF);        // USART2 SPI0
    swm0.PINASSIGN4.set(0x041C1200);        // SPI0
    swm0.PINASSIGN5.set(0xFFFF2122);        // SPI0/1
    swm0.PINASSIGN6.set(0x0BFFFFFF);        // SPI1 I2C0
    swm0.PINASSIGN7.set(0xFFFFFF0A);        // I2C0 I3C0
    swm0.PINASSIGN8.set(0xFFFF09FF);        // ACMP0 CLKOUT (on TP9) GPIO_INT_BMAT 
    swm0.FTM_PINASSIGN0.set(0x507FD431);    // FTM0/1
    swm0.PINENABLE0.set(0xFFFF081F);        // enable ADC0..3, CLKIN, XTALIN, RESET and SWD

    auto &inputmux = *i_INPUTMUX.registers; // INPUTMUX register set

    return 0;
}

void *__dso_handle = nullptr;

extern "C" {

void _exit(int status) {
    while(1);   // _exit doesn't return
}

void _init(void) {}
void _fini(void) {}

/** Reset entry point.
 * Sets up a simple runtime environment and initializes the C/C++ library.
 */
void ResetISR(void) {
    sysinit();              // early system initialization to make memory usable
    startup_meminit();      // initialize .data and .bss
    __libc_init_array();    // C++ library initialization
    _exit(main());          // _exit() never returns
}

///// Newlib dummy stubs

__attribute__((noreturn)) void panic(char const *msg) {
    // Replace with your MCU-specific panic handler (e.g., LED blink, halt, etc.)
    while (1) { /* trap */ }
}

void *_sbrk_r(struct _reent *r, ptrdiff_t incr) {
    panic("_sbrk_r called: dynamic memory not supported");
    return (void*)-1; // never reached
}
_ssize_t _write_r(struct _reent *r, int fd, const void *ptr, size_t len) {
    return 0; // can't write anything
}
_ssize_t _read_r(struct _reent *r, int fd, void *ptr, size_t len) {
    return 0; // no input available
}
int _close_r(struct _reent *r, int fd) {
    return -1;  // error    
}
_off_t _lseek_r(struct _reent *r, int fd, _off_t off, int whence) {
    return -1;  // error    
}
int _fstat_r(struct _reent *r, int fd, struct stat *st) {
    return -1;  // error    
}       
int _kill_r(struct _reent *r, int pid, int sig) {
    return -1;  // error    
}
int _getpid_r(struct _reent *r) {
    return 1;   // dummy PID    
}   
int _isatty_r(struct _reent *r, int fd) {
    return 0;   // no TTY    
}

} // extern "C"

/** @} */
