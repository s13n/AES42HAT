/** @file
 * driver for the LPC8 WKT
 * @addtogroup LPC865_wkt
 * @ingroup LPC865
 * @{
 */
#include "wkt_drv.hpp"
#include "handler.hpp"
#include <bit>


void lpc865::Wkt::isr() {
    auto &hw = *in_.registers;
    hw.CTRL.set(hw.CTRL.get());     // clear ALARMFLAG if set
    if (hdl_)
        hdl_->post();
}

void lpc865::Wkt::start(uint32_t count, Handler &hdl) {
    hdl_ = &hdl;
    in_.registers->COUNT.set(count);
}

lpc865::Wkt::Wkt(WKT::Integration const &in, Parameters const &par)
    : in_{in}
    , hdl_{nullptr}
{
    auto &hw = *in_.registers;
    hw.CTRL = CTRL{ .CLKSEL = par.clksel, .ALARMFLAG = 1, .CLEARCTR = CLEAR_THE_COUNTER, .SEL_EXTCLK = par.extclk };

    insert(in_.exWKT);
}

/** @}*/
