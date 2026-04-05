/** @file
 * SRC4392 driver.
 * @addtogroup SRC4392
 * @{
 */

module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
module src4392_drv;
import spi_drv;
import SRC4392;


namespace src4392 {

Src4392::Src4392(SRC4392::Intgr const &in, Handler *hdl)
    : entry_{
        .par = {
            .cmd = {
                .pu = lpc865::Spi::pu1S1S1S,
                .maxHz = lpc865::Spi::mHz33,
                .write = 1,
                .dummy = 8
            },
            .type = lpc865::Spi::spiMode0,
            .sel = 1U << in.addr
        },
        .buf = nullptr,
        .size = 0,
        .hdl = hdl
    }
{
}

std::byte *Src4392::getPtr(uint8_t addr, std::byte &page) {
    if (addr == 0x7F)
        return &page;
    switch (static_cast<uint8_t>(page) & 0x03) {
    case 0:
        if (addr == 0)
            return nullptr;
        if (addr <= 0x33)
            return &regs_[addr - 1];
        return nullptr;
    case 1:
        if (addr <= 0x2F)
            return &rxcs_[addr];
        if (addr < 0x40)
            return nullptr;
        if (addr <= 0x6F)
            return &rxu_[addr-0x40];
        return nullptr;
    case 2:
        if (addr <= 0x2F)
            return &txcs_[addr];
        if (addr < 0x40)
            return nullptr;
        if (addr <= 0x6F)
            return &txu_[addr-0x40];
        return nullptr;
    default:
        return nullptr;
    }
}

uint64_t Src4392::update(std::span<std::byte const> buf, std::span<std::byte> internal) {
    size_t size = std::min(buf.size(), internal.size());
    uint64_t res{0};
    for (size_t i = 0; i < size; ++i) {
        res |= uint64_t(buf[i] != internal[i]) << i;
        internal[i] = buf[i];
    }
    return res;
}

void Src4392::rdwr(lpc865::SpiQueue &spiq, std::span<std::byte> buf, uint8_t reg) {
    entry_.buf = buf.data();
    entry_.size = buf.size();
    entry_.par.cmd.read = reg >> 7;
    entry_.par.cmd.ins = reg;
    spiq.enqueue(entry_);
}

} //!@} namespace
