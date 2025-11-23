/** @file
 * support for the LPC8 SPI block.
 * @addtogroup LPC865_spi
 * @ingroup LPC865
 * @{
 */
#include "spi_drv.hpp"
#include "dma_drv.hpp"


bool lpc865::Spi::target(Parameters const &par, Event *ev) {
    if (par.sel & 0xF0)
        return false;
    auto &hw = *in_.registers;
    auto txctl = hw.TXCTL.get();
    txctl.TXSSEL0_N = !(par.sel & 0x01);
    txctl.TXSSEL1_N = !(par.sel & 0x02);
    txctl.TXSSEL2_N = !(par.sel & 0x04);
    txctl.TXSSEL3_N = !(par.sel & 0x08);
    txctl.EOT = 0;
    txctl.EOF = 0;
    txctl.RXIGNORE = !par.cmd.read;
    txctl.LEN = 7;  // 8 bit data length
    hw.TXCTL.set(txctl);
    return true;
}

ptrdiff_t lpc865::Spi::transfer(void *buf, size_t size, uint32_t speed) {
    auto &hw = *in_.registers;
    if (dma_) {
        DMA::CFG cfg{ .PERIPHREQEN=1 };
        DMA::XFERCFG xferRx{ .CFGVALID=1, .SETINTA=1, .WIDTH=BIT_8, .SRCINC=SRCINC::NO_INCREMENT, .DSTINC=DSTINC::WIDTH_X_1, .XFERCOUNT=uint16_t(size - 1) };
        auto rxChan = dma_->setup(in_.rx_req, cfg, xferRx, reinterpret_cast<intptr_t>(&hw.RXDAT));
        DMA::XFERCFG xferTx{ .CFGVALID=1, .WIDTH=BIT_8, .SRCINC=SRCINC::WIDTH_X_1, .DSTINC=DSTINC::NO_INCREMENT, .XFERCOUNT=uint16_t(size - 1) };
        auto txChan = dma_->setup(in_.tx_req, cfg, xferTx, reinterpret_cast<intptr_t>(&hw.TXDAT));
        return 0;
    } else {
        SPI::TXCTL txctl = hw.TXCTL.get();
        auto dst = txctl.RXIGNORE ? nullptr : static_cast<uint8_t *>(buf);
        auto src = static_cast<uint8_t const *>(buf);
        auto end = src + size;
        SPI::STAT stat = hw.STAT.get();
        hw.STAT.set(stat);      // clear some flags
        while (src < end || (dst && dst < end)) {
            do stat = hw.STAT.get();
            while (!stat.RXRDY && !(src < end && stat.TXRDY));
            if (dst && dst < end && stat.RXRDY) {
                uint32_t d = hw.RXDAT.val();
                *dst++ = uint8_t(d);
            }
            if (src < end && stat.TXRDY) {
                hw.TXDAT.set(*src++);
                if (src == end) {
                    txctl = hw.TXCTL.get();
                    txctl.EOT = 1;
                    hw.TXCTL.set(txctl);
                }
            }
        }
        return size;
    }
}

auto lpc865::Spi::status() const -> Status {
    return idle;
}

lpc865::Spi::Spi(SPI::Integration const &in, DmaBase *dma)
    : in_{in}
    , dma_{dma}
{
    auto &hw = *in_.registers;
    hw.DIV.set(11);  // divide by 12
    hw.CFG.set(SPI::CFG{ .ENABLE = 1, .MASTER = 1 });
}

/** @}*/
