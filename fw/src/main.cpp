/** @file
 * main function for the AES42HAT
 */

#include "clocks.hpp"
#include "ftm_drv.hpp"
#include "i2c_tgt_drv.hpp"
#include "pint_drv.hpp"
#include "spi_drv.hpp"
#include "usart_drv.hpp"
#include "wkt_drv.hpp"
#include "clkmgr.hpp"
#include "channel.hpp"
#include "handler.hpp"
#include <stdint.h>
#include <string_view>

using namespace lpc865;

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

static clocktree::ClockTree<Clocks> clktree;
static Pint pint{ i_PINT };             // Pin interrupt driver
static Spi spi0{ i_SPI0, nullptr };     // SRC4392 control communication
static Spi spi1{ i_SPI1, nullptr };     // Wordclock generation
static Ftm ftm0{ i_FTM0, ftm0par };     // Wordclock phase measurements
static Ftm ftm1{ i_FTM1, ftm1par };     // Mode 2 remote control pulse generation
static Usart usart0{ i_USART0 };        // Host communication USART
static Usart usart1{ i_USART1 };        // Console mode receive USART (RX only)
static Usart usart2{ i_USART2 };        // Mode 3 remote control USART (TX only)

static Channel chan[4] = {
    {spi0, ftm0, pint, 0, 0},
    {spi0, ftm0, pint, 1, 1},
    {spi0, ftm0, pint, 2, 2},
    {spi0, ftm0, pint, 3, 3}
};

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

static Wkt wkt{i_WKT, {1, 0}};

void print(std::string_view buf) {
    do buf.remove_prefix(usart0.send(buf.data(), buf.size()));
    while (!buf.empty());
}

void setActivityLED(bool act) {
    i_GPIO.registers->B[1].B_[7].set(act);
}
/*
class Blinky : public Handler {
    Wkt &wkt_;
    HwPtr<GPIO::GPIO volatile> gpio_;   // GPIO register set
    uint32_t interval_;
public:
    explicit Blinky(Wkt &wkt)
        : wkt_{wkt}
        , gpio_{i_GPIO.registers}
        , interval_{}
    {
        gpio_->DIRSET[1].set(1 << 7);
    }

    void set(uint32_t interval) {
        interval_ = interval;
        act();
    }

    void act() override {
        gpio_->NOT[1].set(1 << 7);      // LED toggle
        wkt_.start(interval_, *this);
    }
};

static Blinky blinky{wkt};
*/
int main() {
    i_GPIO.registers->DIRSET[1].set(1 << 7);

    clktree.register_fields[1].set(static_cast<Clocks*>(&clktree), 60000000);

//    blinky.set(clktree.getFrequency(Clocks::S::wkt_counter_clk) / 2);

    print("AES42HAT\n");

    Clkmgr clkmgr(ftm0, pint, chan, 0, 4);

    for (size_t i=0; i<4; ++i) {
        chan[i].updateSrcCtrl();
    }

    Handler::run();

    return 0;
}
