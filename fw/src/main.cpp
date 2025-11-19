/** @file
 * main function for the AES42HAT
 */

#include "WKT.hpp"
#include "WWDT.hpp"
#include "clocks.hpp"
#include "ftm_drv.hpp"
#include "i2c_tgt_drv.hpp"
#include "spi_drv.hpp"
#include "usart_drv.hpp"
#include "channel.hpp"
#include "handler.hpp"
#include <stdint.h>


using namespace lpc865;

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
static Spi spi0{ i_SPI0, nullptr };     // SRC4392 control communication
static Spi spi1{ i_SPI1, nullptr };     // Wordclock generation
static Ftm ftm0{ i_FTM0, ftm0par };     // Wordclock phase measurements
static Ftm ftm1{ i_FTM1, ftm1par };     // Mode 2 remote control pulse generation
static Usart usart0{ i_USART0 };        // Host communication USART
static Usart usart1{ i_USART1 };        // Console mode receive USART (RX only)
static Usart usart2{ i_USART2 };        // Mode 3 remote control USART (TX only)

static Channel chan[4] = { {0, spi0}, {1, spi0}, {2, spi0}, {3, spi0} };

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

extern "C" void I2C0isr() {
    i2c0.isr();
}

extern "C" void USART0isr() {
    usart0.isr();
}

extern "C" void USART1isr() {
    usart1.isr();
}

extern "C" void USART2isr() {
    usart2.isr();
}

class Blinky : public Handler {
    HwPtr<GPIO::GPIO volatile> gpio_;   // GPIO register set
    HwPtr<WKT::WKT volatile> wkt_;      // WKT register set
public:
    Blinky()
        : gpio_{i_GPIO.registers}
        , wkt_{i_WKT.registers}
    {
        gpio_->DIRSET[1].set(1 << 7);
    }

    void act() override {
        gpio_->NOT[1].set(1 << 7);      // LED toggle
        wkt_->CTRL.set(6);
        wkt_->COUNT.set(5000);
    }

    void isr() {
        post();
    }
};

static Blinky blinky;

extern "C" void WKTisr() {
    blinky.isr();
}

int main() {
    uint32_t f = clktree.getFrequency(Clocks::S::system_clk);
    clktree.register_fields[1].set(static_cast<Clocks*>(&clktree), 60000000);
    f = clktree.getFrequency(Clocks::S::system_clk);

    Handler::run();

    return 0;
}
