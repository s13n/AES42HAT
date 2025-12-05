/** @file
 * support for the LPC8 SPI block.
 * @addtogroup LPC865_spi
 * @ingroup LPC865
 * @{
 */
#include "spi_drv.hpp"
#include "dma_drv.hpp"
#include "SPI.hpp"


bool lpc865::Spi::target(Parameters const &par, Handler *hdl) {
    if (par.sel & 0xF0)
        return false;
    hdl_ = hdl;
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
    if (dma_) {
        Dma::Per tx{ .chan = in_.tx_req, .width=0, .dest=1 };
        if (!dma_->setup(tx, reinterpret_cast<uintptr_t>(&hw.TXDAT), this))
            return false;
        if (par.cmd.read) {
            Dma::Per rx{ .chan = in_.rx_req, .width=0, .dest=0 };
            if (!dma_->setup(rx, reinterpret_cast<uintptr_t>(&hw.RXDAT), this))
                return false;
        }
    }
    return true;
}

ptrdiff_t lpc865::Spi::transfer(void *buf, size_t size, uint32_t speed) {
    auto &hw = *in_.registers;
    auto stat = hw.STAT.get();
    if (!stat.MSTIDLE)
        return -1;
    hw.STAT = stat;         // clear any interrupt there may be
    if (dma_) {
        bool read = hw.TXCTL.get().RXIGNORE == 0;
        if (read) {
            Dma::Mem rx{ .chan = in_.rx_req, .inc=1, .setintA=1 };
            if (!dma_->start(rx, buf, size))
                return -1;
        }
        Dma::Mem tx{ .chan = in_.tx_req, .inc=1, .setintA=!read };
        if (!dma_->start(tx, buf, size))
            return -1;
        hw.INTENSET = INTENSET{ .SSDEN=1 };
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
        if (hdl_)
            hdl_->post();
        return size;
    }
}

auto lpc865::Spi::status() const -> Status {
    return idle;
}

lpc865::Spi::Spi(SPI::Integration const &in, Dma *dma)
    : in_{in}
    , dma_{dma}
{
    auto &hw = *in_.registers;
    hw.DIV.set(11);  // divide by 12
    hw.DLY = DLY{ .PRE_DELAY = 1, .POST_DELAY = 1 };
    hw.CFG = SPI::CFG{ .ENABLE = 1, .MASTER = 1 };
    insert(in_.exSPI);
}

// This gets called when the DMA controller generated an interrupt for the SPI-assigned channels.
void lpc865::Spi::act() {
    auto &hw = *in_.registers;
    hw.STAT = STAT{ .ENDTRANSFER=1 };
}

// This gets called when the SPI generates an interrupt request.
void lpc865::Spi::isr() {
    auto &hw = *in_.registers;
    auto stat = hw.STAT.get();
    hw.STAT = stat;     // clear pending interrupts
    if (stat.SSD && hdl_)
        hdl_->post();
}

/** @}*/
