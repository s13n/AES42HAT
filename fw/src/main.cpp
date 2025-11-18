/** @file
 * main function for the AES42HAT
 */

#include "WKT.hpp"
#include "WWDT.hpp"
#include "clocks.hpp"
#include "ftm_drv.hpp"
#include "spi_drv.hpp"
#include "src4392_drv.hpp"
#include <stdint.h>


constexpr std::byte operator""_y(unsigned long long value) {
    return static_cast<std::byte>(value);
}

using namespace lpc865;

static std::array<std::byte, 52> const srcInitData = {
    0x00_y,
    0x3F_y, // Register 01: Power-Down and Reset
    0x00_y, // Register 02: Global Interrupt Status (Read-Only)
    0x31_y, // Register 03: Port A Control Register 1
    0x00_y, // Register 04: Port A Control Register 2
    0x31_y, // Register 05: Port B Control Register 1
    0x00_y, // Register 06: Port B Control Register 2
    0x00_y, // Register 07: Transmitter Control Register 1
    0x10_y, // Register 08: Transmitter Control Register 2
    0x00_y, // Register 09: Transmitter Control Register 3
    0x00_y, // Register 0A: SRC and DIT Status (Read-Only)
    0x00_y, // Register 0B: SRC and DIT Interrupt Mask Register
    0x00_y, // Register 0C: SRC and DIT Interrupt Mode Register
    0x08_y, // Register 0D: Receiver Control Register 1
    0x00_y, // Register 0E: Receiver Control Register 2
    0x12_y, // Register 0F: Receiver PLL1 Configuration Register 1
    0x00_y, // Register 10: Receiver PLL1 Configuration Register 2
    0x00_y, // Register 11: Receiver PLL1 Configuration Register 3
    0x00_y, // Register 12: Non-PCM Audio Detection Status Register (Read-Only)
    0x00_y, // Register 13: Receiver Status Register 1 (Read-Only)
    0x00_y, // Register 14: Receiver Status Register 2 (Read-Only)
    0x00_y, // Register 15: Receiver Status Register 3 (Read-Only)
    0x01_y, // Register 16: Receiver Interrupt Mask Register 1
    0x00_y, // Register 17: Receiver Interrupt Mask Register 2
    0x00_y, // Register 18: Receiver Interrupt Mode Register 1
    0x00_y, // Register 19: Receiver Interrupt Mode Register 2
    0x00_y, // Register 1A: Receiver Interrupt Mode Register 3
    0x01_y, // Register 1B: General-Purpose Output 1 (GPO1) Control Register
    0x00_y, // Register 1C: General-Purpose Output 2 (GPO2) Control Register
    0x0E_y, // Register 1D: General-Purpose Output 3 (GPO3) Control Register
    0x09_y, // Register 1E: General-Purpose Output 4 (GPO4) Control Register
    0x00_y, // Register 1F: Q-Channel Sub-Code Data Register 1 (Read-Only), Bits[7:0], Control and Address
    0x00_y, // Register 20: Q-Channel Sub-Code Data Register 2 (Read-Only), Bits[15:8], Track
    0x00_y, // Register 21: Q-Channel Sub-Code Data Register 3 (Read-Only), Bits[23:16], Index
    0x00_y, // Register 22: Q-Channel Sub-Code Data Register 4 (Read-Only), Bits[31:24], Minutes
    0x00_y, // Register 23: Q-Channel Sub-Code Data Register 5 (Read-Only), Bits[39:32], Seconds
    0x00_y, // Register 24: Q-Channel Sub-Code Data Register 6 (Read-Only), Bits[47:40], Frame
    0x00_y, // Register 25: Q-Channel Sub-Code Data Register 7 (Read-Only), Bits[55:48], Zero
    0x00_y, // Register 26: Q-Channel Sub-Code Data Register 8 (Read-Only), Bits[63:56], AMIN
    0x00_y, // Register 27: Q-Channel Sub-Code Data Register 9 (Read-Only), Bits[71:64], ASEC
    0x00_y, // Register 28: Q-Channel Sub-Code Data Register 10 (Read-Only), Bits[79:72], AFRAME
    0x00_y, // Register 29: Burst Preamble PC High-Byte Status Register (Read-Only)
    0x00_y, // Register 2A: Burst Preamble PC Low-Byte Status Register (Read-Only)
    0x00_y, // Register 2B: Burst Preamble PD High-Byte Status Register (Read-Only)
    0x00_y, // Register 2C: Burst Preamble PD Low-Byte Status Register (Read-Only)
    0x02_y, // Register 2D: SRC Control Register 1
    0x00_y, // Register 2E: SRC Control Register 2
    0x00_y, // Register 2F: SRC Control Register 3
    0x00_y, // Register 30: SRC Control Register 4
    0x00_y, // Register 31: SRC Control Register 5
    0x00_y, // Register 32: SRC Ratio Readback Register (Read-Only)
    0x00_y, // Register 33: SRC Ratio Readback Register (Read-Only)
};

static src4392::Integration const i_src4392A{ .addr = 0, .cpm = 0, .src_present=1 };
static src4392::Integration const i_src4392B{ .addr = 1, .cpm = 0, .src_present=1 };
static src4392::Integration const i_src4392C{ .addr = 2, .cpm = 0, .src_present=1 };
static src4392::Integration const i_src4392D{ .addr = 3, .cpm = 0, .src_present=1 };

static lpc865::Ftm::Parameters const ftm0par{ .ps=0, .clks=3, .mod=12287
    , .ch0inv=0
    , .ch0=::Ftm::pwmNeg
    , .ch2=::Ftm::captureNeg
    , .ch3=::Ftm::captureNeg
    , .ch4=::Ftm::captureNeg
    , .ch5=::Ftm::captureNeg
};
static lpc865::Ftm::Parameters const ftm1par{ .ps=1, .clks=1, .mod=40000
    , .ch0inv=1
    , .ch1inv=1
    , .ch2inv=1
    , .ch3inv=1
    , .ch0=::Ftm::pwmNeg
    , .ch1=::Ftm::pwmNeg
    , .ch2=::Ftm::pwmNeg
    , .ch3=::Ftm::pwmNeg
};

static clocktree::ClockTree<Clocks> clktree;

int main() {
    Spi spi0{ i_SPI0, nullptr };
    std::array<src4392::Src4392, 4> src{ i_src4392A, i_src4392B, i_src4392C, i_src4392D };
    Ftm ftm0{ i_FTM0, ftm0par };
    Ftm ftm1{ i_FTM1, ftm1par };
    
    uint32_t f = clktree.getFrequency(Clocks::S::ftm0fclk);
    clktree.register_fields[4].set(static_cast<Clocks*>(&clktree), 3072000);
    f = clktree.getFrequency(Clocks::S::ftm0fclk);

    for (auto &s : src) {
        s.updateRegs(srcInitData);
        s.writeRegs(spi0);
    }
    
    auto &gpio = *i_GPIO.registers;     // GPIO register set
    auto &wkt = *i_WKT.registers;       // WKT register set

    gpio.DIRSET[1].set(1 << 7);
    while (1) {
        gpio.NOT[1].set(1 << 7); // LED toggle

        wkt.CTRL.set(6);
        wkt.COUNT.set(5000);
        while (wkt.CTRL.get().ALARMFLAG == NO_TIME_OUT);
    }
    return 0;
}
