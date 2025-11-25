/** @file
 * AES42HAT clock management.
 * @addtogroup AES42HAT_clk
 * @ingroup AES42HAT
 * @{
 */
#include "clkmgr.hpp"
#include "channel.hpp"
#include "ftm_drv.hpp"
#include "pint_drv.hpp"
#include <string_view>


extern void print(std::string_view);

void Clkmgr::act() {
    channels_[0].handleTxBlock();
    channels_[1].handleTxBlock();
    channels_[2].handleTxBlock();
    channels_[3].handleTxBlock();
    pint_.enable(irq_);
}

void Clkmgr::isr() {
    pint_.disable(irq_);
    post();
}

Clkmgr::Clkmgr(lpc865::Ftm &ftm, lpc865::Pint &pint, Channel *channels, uint8_t tch, uint8_t irq)
    : tch_{tch}
    , irq_{irq}
    , ftm_{ftm}
    , pint_{pint}
    , channels_{channels}
{
    pint_.attach(irq_, *this);
}

/** @}*/
