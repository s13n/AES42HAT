/** @file
 * driver for the LPC8 USART block.
 * @addtogroup LPC865_usart
 * @ingroup LPC865
 * @{
 */
#include "usart_drv.hpp"
#include <span>

size_t lpc865::Usart::send(void const *buf, size_t size) {
    auto &hw = *in_.registers;
    size_t res = 0;
    if (auto *p = static_cast<uint8_t const *>(buf))
        while ((res < size) && (hw.STAT.get().TXRDY != 0))
            hw.TXDAT = p[res++];
    return res;
}

void lpc865::Usart::isr() {
}

lpc865::Usart::Usart(USART::Integration const &in)
    : in_{in}
{
    auto &hw = *in_.registers;
    CFG cfg = { .DATALEN = BIT_8 };
    hw.CFG = cfg;
    hw.CTL.set(0);
    hw.STAT.set(hw.STAT.get());
    hw.INTENCLR.set(hw.INTENSET.val());
    hw.BRG = BRG{ .BRGVAL = 3 };
    hw.OSR = OSR{ .OSRVAL = 15 };
    cfg.ENABLE = 1;
    hw.CFG = cfg;
}

/** @}*/
