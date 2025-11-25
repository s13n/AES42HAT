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
    std::array<std::byte, 3> buf{ std::byte(0x7F), std::byte(0), page_ };
    write(spi, buf);
}

void Src4392::writeRegs(lpc865::Spi &spi) {
    std::array<std::byte, sizeof(regs_) + 2> buf{ std::byte(0x01), std::byte(0) };
    std::copy(regs_.begin(), regs_.end(), buf.begin() + 2);
    write(spi, buf);
}

void Src4392::writeCS(lpc865::Spi &spi) {
    std::array<std::byte, sizeof(txcs_) + 2> buf{ std::byte(0x00), std::byte(0) };
    std::copy(txcs_.begin(), txcs_.end(), buf.begin() + 2);
    write(spi, buf);
}

void Src4392::writeU(lpc865::Spi &spi) {
    std::array<std::byte, sizeof(txu_) + 2> buf{ std::byte(0x40), std::byte(0) };
    std::copy(txu_.begin(), txu_.end(), buf.begin() + 2);
    write(spi, buf);
}

uint64_t Src4392::readRegs(lpc865::Spi &spi) {
    std::array<std::byte, sizeof(regs_) + 2> buf{ std::byte(0x81), std::byte(0) };
    read(spi, buf);
    return update({buf.data() + 2, buf.data() + buf.size() - 2}, regs_);
}

void Src4392::readTxStatus(lpc865::Spi &spi) {
    std::array<std::byte, 3> srcdit{ std::byte(0x8A), std::byte(0) };
    read(spi, srcdit);
    update({srcdit.data() + 2, srcdit.data() + srcdit.size() - 2}, std::span(regs_).subspan(9,1));
    std::array<std::byte, 4> ratio{ std::byte(0xB2), std::byte(0) };
    read(spi, ratio);
    update({ratio.data() + 2, ratio.data() + ratio.size() - 2}, std::span(regs_).subspan(49,2));
}

void Src4392::readRxStatus(lpc865::Spi &spi) {
    std::array<std::byte, 6> dir{ std::byte(0x92), std::byte(0) };
    read(spi, dir);
    update({dir.data() + 2, dir.data() + dir.size() - 2}, std::span(regs_).subspan(17,4));
    std::array<std::byte, 16> subc{ std::byte(0x9F), std::byte(0) };
    read(spi, subc);
    update({subc.data() + 2, subc.data() + subc.size() - 2}, std::span(regs_).subspan(30,14));
}

uint64_t Src4392::readCS(lpc865::Spi &spi) {
    std::array<std::byte, sizeof(rxcs_) + 2> buf{ std::byte(0x80), std::byte(0) };
    read(spi, buf);
    return update({buf.data() + 2, buf.data() + buf.size() - 2}, rxcs_);
}

uint64_t Src4392::readU(lpc865::Spi &spi) {
    std::array<std::byte, sizeof(rxu_) + 2> buf{ std::byte(0xC0), std::byte(0) };
    read(spi, buf);
    return update({buf.data() + 2, buf.data() + buf.size() - 2}, rxu_);
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

void Src4392::read(lpc865::Spi &spi, std::span<std::byte> buf) {
    lpc865::Spi::Parameters p{
        .cmd = {
            .pu = lpc865::Spi::pu1S1S1S,
            .maxHz = lpc865::Spi::mHz33,
            .read = 1,
            .write = 1,
        },
        .type = lpc865::Spi::spiMode0,
        .noins = 1,
        .sel = 1U << in_.addr
    };
    spi.target(p, nullptr);
    spi.transfer(buf.data(), buf.size());
}

void Src4392::write(lpc865::Spi &spi, std::span<std::byte> buf) {
    lpc865::Spi::Parameters p{
        .cmd = {
            .pu = lpc865::Spi::pu1S1S1S,
            .maxHz = lpc865::Spi::mHz33,
            .write = 1,
        },
        .type = lpc865::Spi::spiMode0,
        .noins = 1,
        .sel = 1U << in_.addr
    };
    spi.target(p, nullptr);
    spi.transfer(buf.data(), buf.size());
}

} //!@} namespace
