/** @file
 * AES42HAT clock management.
 * @addtogroup AES42HAT_clk
 * @ingroup AES42HAT
 * @{
 */
#include "clkmgr.hpp"
#include "channel.hpp"
#include <string_view>


extern void print(std::string_view);

void Clkmgr::act() {
    CORO_REENTER(coro_) {
        CORO_YIELD channels_[0].handleTxBlock();
        CORO_YIELD channels_[1].handleTxBlock();
        CORO_YIELD channels_[2].handleTxBlock();
        CORO_YIELD channels_[3].handleTxBlock();
        pint_.enable(irq_, 1);
    }
}

void Clkmgr::isr() {
    pint_.disable(irq_);
    post();
}

Clkmgr::Clkmgr(lpc865::Pint &pint, Channel *channels, uint8_t irq)
    : irq_{irq}
    , pint_{pint}
    , channels_{channels}
{
    pint_.attach(irq_, 1, *this);
}


void ChannelManagement::act() {
    print("%");
    CORO_REENTER(coro_) {
        CORO_YIELD channels_[0].updateSrcCtrl();
        CORO_YIELD channels_[1].updateSrcCtrl();
        CORO_YIELD channels_[2].updateSrcCtrl();
        CORO_YIELD channels_[3].updateSrcCtrl();
    }
}

/** @}*/
