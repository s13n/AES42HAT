/** @file
 * SRC4392 driver.
 * @addtogroup SRC4392
 * @{
 */

#include "src4392_drv.hpp"
#include "spi_drv.hpp"
#include <algorithm>

namespace src4392 { 
    
Src4392::Src4392(SRC4392::Integration const &in)
    : in_(in)
{
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

void Src4392::writeRegs(lpc865::Spi &spi) {
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
    std::array<std::byte, sizeof(regs_) + 2> buf{};
    std::copy(regs_.begin(), regs_.end(), buf.data() + 2);
    spi.transfer(buf.data(), buf.size());
}

} //!@} namespace
