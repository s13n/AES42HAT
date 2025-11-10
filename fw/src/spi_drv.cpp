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
    txctl.EOT = 1;
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
        auto src = static_cast<uint8_t const *>(buf);
        auto srcend = src + size;
        auto dst = static_cast<uint8_t *>(buf);
        auto dstend = dst + size;
        while (src < srcend || dst < dstend) {
            auto stat = hw.STAT.get();
            while (!stat.TXRDY && !stat.RXRDY)
                ;
            if (dst < dstend && stat.RXRDY)
                *dst++ = hw.RXDAT.val();
            if (src < srcend && stat.TXRDY)
                hw.TXDAT.set(*src++);
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
    hw.DIV.set(1);  // divide by 2
    hw.CFG.set(SPI::CFG{ .ENABLE = 1, .MASTER = 1 });
}

/** @}*/
