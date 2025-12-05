/** @file
 * SRC4392 driver.
 * @addtogroup SRC4392
 * @{
 */

#include "src4392_drv.hpp"
#include "spi_drv.hpp"
#include "SRC4392.hpp"
#include <algorithm>
#include <alloca.h>


namespace src4392 { 
    
Src4392::Src4392(SRC4392::Integration const &in)
    : in_(in)
{
}

void Src4392::switchPage(lpc865::Spi &spi, uint8_t page) {
    page_ = std::byte(page);
    write(spi, 0x7F, { &page_, 1 });
}

void Src4392::writeRegs(lpc865::Spi &spi) {
    write(spi, 0x01, regs_);
}

void Src4392::writeCS(lpc865::Spi &spi) {
    write(spi, 0x00, txcs_);
}

void Src4392::writeU(lpc865::Spi &spi) {
    write(spi, 0x40, txu_);
}

void Src4392::readRegs(lpc865::Spi &spi) {
    read(spi, 0x81, regs_);
}

void Src4392::readTxStatus(lpc865::Spi &spi) {
    read(spi, 0x8A, std::span(regs_).subspan(9,1));
    read(spi, 0xB2, std::span(regs_).subspan(49,2));
}

void Src4392::readRxStatus(lpc865::Spi &spi) {
    read(spi, 0x92, std::span(regs_).subspan(17,4));
    read(spi, 0x9F, std::span(regs_).subspan(30,14));
}

void Src4392::readCS(lpc865::Spi &spi) {
    read(spi, 0x80, rxcs_);
}

void Src4392::readU(lpc865::Spi &spi) {
    read(spi, 0xC0, rxu_);
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

void Src4392::read(lpc865::Spi &spi, uint8_t reg, std::span<std::byte> buf) {
    lpc865::Spi::Parameters p{
        .cmd = {
            .pu = lpc865::Spi::pu1S1S1S,
            .maxHz = lpc865::Spi::mHz33,
            .read = 1,
            .write = 1,
            .dummy = 8,
            .ins = reg
        },
        .type = lpc865::Spi::spiMode0,
        .sel = 1U << in_.addr
    };
    spi.target(p, nullptr);
    spi.transfer(buf.data(), buf.size());
}

void Src4392::write(lpc865::Spi &spi, uint8_t reg, std::span<std::byte> buf) {
    lpc865::Spi::Parameters p{
        .cmd = {
            .pu = lpc865::Spi::pu1S1S1S,
            .maxHz = lpc865::Spi::mHz33,
            .write = 1,
            .dummy = 8,
            .ins = reg
        },
        .type = lpc865::Spi::spiMode0,
        .sel = 1U << in_.addr
    };
    spi.target(p, nullptr);
    spi.transfer(buf.data(), buf.size());
}

} //!@} namespace
