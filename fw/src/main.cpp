/** @file
 * main function for the AES42HAT
 */

#include "dma_drv.hpp"
#include "ftm_drv.hpp"
#include "i2c_tgt_drv.hpp"
#include "pint_drv.hpp"
#include "spi_queue.hpp"
#include "usart_drv.hpp"
#include "wkt_drv.hpp"
#include "clkmgr.hpp"
#include "channel.hpp"
#include "handler.hpp"
#include "LPC865.hpp"
#include "LPC865_clocks.hpp"
#include <string_view>

using namespace lpc865;

constexpr std::byte operator""_y(unsigned long long value) {
    return static_cast<std::byte>(value);
}

static std::initializer_list<std::byte> const srcInitData0 = {
    0x3F_y, // Register 01: Power-Down and Reset
    0x00_y, // Register 02: Global Interrupt Status (Read-Only)
    0x39_y, // Register 03: Port A Control Register 1
    0x03_y, // Register 04: Port A Control Register 2
    0x31_y, // Register 05: Port B Control Register 1
    0x00_y, // Register 06: Port B Control Register 2
    0x7D_y, // Register 07: Transmitter Control Register 1
    0x20_y, // Register 08: Transmitter Control Register 2
    0x01_y, // Register 09: Transmitter Control Register 3
    0x00_y, // Register 0A: SRC and DIT Status (Read-Only)
    0x00_y, // Register 0B: SRC and DIT Interrupt Mask Register
    0x00_y, // Register 0C: SRC and DIT Interrupt Mode Register
    0x08_y, // Register 0D: Receiver Control Register 1
    0x00_y, // Register 0E: Receiver Control Register 2
    0x22_y, // Register 0F: Receiver PLL1 Configuration Register 1
    0x00_y, // Register 10: Receiver PLL1 Configuration Register 2
    0x00_y, // Register 11: Receiver PLL1 Configuration Register 3
    0x00_y, // Register 12: Non-PCM Audio Detection Status Register (Read-Only)
    0x00_y, // Register 13: Receiver Status Register 1 (Read-Only)
    0x00_y, // Register 14: Receiver Status Register 2 (Read-Only)
    0x00_y, // Register 15: Receiver Status Register 3 (Read-Only)
    0x01_y, // Register 16: Receiver Interrupt Mask Register 1
    0x00_y, // Register 17: Receiver Interrupt Mask Register 2
    0x02_y, // Register 18: Receiver Interrupt Mode Register 1
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

static std::initializer_list<std::byte> const srcInitData = {
    0x3F_y, // Register 01: Power-Down and Reset
    0x00_y, // Register 02: Global Interrupt Status (Read-Only)
    0x31_y, // Register 03: Port A Control Register 1
    0x00_y, // Register 04: Port A Control Register 2
    0x31_y, // Register 05: Port B Control Register 1
    0x00_y, // Register 06: Port B Control Register 2
    0x79_y, // Register 07: Transmitter Control Register 1
    0x20_y, // Register 08: Transmitter Control Register 2
    0x01_y, // Register 09: Transmitter Control Register 3
    0x00_y, // Register 0A: SRC and DIT Status (Read-Only)
    0x00_y, // Register 0B: SRC and DIT Interrupt Mask Register
    0x00_y, // Register 0C: SRC and DIT Interrupt Mode Register
    0x08_y, // Register 0D: Receiver Control Register 1
    0x00_y, // Register 0E: Receiver Control Register 2
    0x22_y, // Register 0F: Receiver PLL1 Configuration Register 1
    0x00_y, // Register 10: Receiver PLL1 Configuration Register 2
    0x00_y, // Register 11: Receiver PLL1 Configuration Register 3
    0x00_y, // Register 12: Non-PCM Audio Detection Status Register (Read-Only)
    0x00_y, // Register 13: Receiver Status Register 1 (Read-Only)
    0x00_y, // Register 14: Receiver Status Register 2 (Read-Only)
    0x00_y, // Register 15: Receiver Status Register 3 (Read-Only)
    0x01_y, // Register 16: Receiver Interrupt Mask Register 1
    0x00_y, // Register 17: Receiver Interrupt Mask Register 2
    0x02_y, // Register 18: Receiver Interrupt Mode Register 1
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

static lpc865::Ftm::Parameters const ftm0par{ .ps=3, .clks=1, .mod=0xFFFF
    , .ch = {
        { .mode=::Ftm::capturePos },    // BLS time stamping
        {},                             // unused    
        { .mode=::Ftm::captureNeg },    // INTA time stamping
        { .mode=::Ftm::captureNeg },    // INTB time stamping
        { .mode=::Ftm::captureNeg },    // INTC time stamping
        { .mode=::Ftm::captureNeg }     // INTD time stamping
    }
};
static lpc865::Ftm::Parameters const ftm1par{ .ps=1, .clks=1, .mod=39999
    , .ch = {
        { .mode=::Ftm::pwmNeg, .inv=1 },
        { .mode=::Ftm::pwmNeg, .inv=1 },
        { .mode=::Ftm::pwmNeg, .inv=1 },
        { .mode=::Ftm::pwmNeg, .inv=1 },
    }
};

static Channel::Integration const i_channel[] = {
    { .in={ .addr = 0, .cpm = 0, .src_present=1 }, .irq=0, .tch=2, .rch=0, .init=srcInitData0 },
    { .in={ .addr = 1, .cpm = 0, .src_present=1 }, .irq=1, .tch=3, .rch=0, .init=srcInitData },
    { .in={ .addr = 2, .cpm = 0, .src_present=1 }, .irq=2, .tch=4, .rch=0, .init=srcInitData },
    { .in={ .addr = 3, .cpm = 0, .src_present=1 }, .irq=3, .tch=5, .rch=0, .init=srcInitData }
};

alignas(512) static std::array<Dma::Descriptor, i_DMA0.max_channel+1> dma_descs;

static lpc865::Dma::Parameters const p_dma = {
    .descs = dma_descs.data()
};

static clocktree::ClockTree<Clocks> clktree;
static Dma dma{ i_DMA0, p_dma };            // DMA controller driver
static Usart usart0{ i_USART0 };            // Host communication USART
static Usart usart1{ i_USART1 };            // Console mode receive USART (RX only)
static Usart usart2{ i_USART2 };            // Mode 3 remote control USART (TX only)
static Pint pint{ i_PINT };                 // Pin interrupt driver
static Ftm ftm0{ i_FTM0, ftm0par };         // Wordclock phase measurements
static Ftm ftm1{ i_FTM1, ftm1par };         // Mode 2 remote control pulse generation
static Wkt wkt{ i_WKT, {1, 0} };
static Spi spi0{ i_SPI0, &dma };            // SRC4392 control communication
static SpiQueue spique{ spi0 };             // Handler queue for SPI0
static Spi spi1{ i_SPI1, nullptr };         // Wordclock generation
static Channel chan[4] = {
    { i_channel[0], spique, ftm0, pint },
    { i_channel[1], spique, ftm0, pint },
    { i_channel[2], spique, ftm0, pint },
    { i_channel[3], spique, ftm0, pint }
};
static Clkmgr clkmgr{pint, chan, 4};

// Operational parameters for target mode I2C0
static I2cTarget::Parameters const p_I2C0 = {
    .addr0 = 0x70,
    .dis1 = 1,
    .dis2 = 1,
    .dis3 = 1,
    .qmode = 1,
    .qual0 = 0x73,
    .callbacks = { &chan[0], &chan[1], &chan[2], &chan[3] }
};

static I2cTarget i2c0{ i_I2C0, p_I2C0 };    // Host communication in target mode

void print(std::string_view buf) {
    do buf.remove_prefix(usart0.send(buf.data(), buf.size()));
    while (!buf.empty());
}

void setActivityLED(bool act) {
    i_GPIO.registers->B[1].B_[7].set(act);
}

int main() {
    i_GPIO.registers->DIRSET[1].set(1 << 7);

    clktree.register_fields[1].set(static_cast<Clocks*>(&clktree), 60000000);

    print("AES42HAT\n");

    ChannelManagement mgmt{chan};
    mgmt.post();

    Handler::run();

    return 0;
}
