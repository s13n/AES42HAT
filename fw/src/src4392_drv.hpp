/** @file
 * SRC4392 driver.
 * @addtogroup SRC4392
 * @{
 */
#pragma once

#include "spi_queue.hpp"
#include <array>
#include <cstdint>
#include <span>

class Handler;

namespace src4392 { 

inline namespace SRC4392 {
    struct Integration;
}

/** SRC4392 driver class.
 */     
class Src4392 {
public:    
    Src4392(Integration const &in, Handler *hdl);

    /** Update the registers.
     * @param buf Buffer containing new register data.
     * 
     * The new data is compared with the old data to determine the change mask.
     * The new data then replaces the old. Finally the change mask is returned.
     */
    uint64_t updateRegs(std::span<std::byte const> buf) {
        return update(buf, regs_);
    }

    /** Update the control/status data.
     * @param buf Buffer containing new data.
     * 
     * The new data is compared with the old data to determine the change mask.
     * The new data then replaces the old. Finally the change mask is returned.
     */
    uint64_t updateCS(std::span<std::byte const> buf) {
        return update(buf, rxcs_);
    }

    /** Update the user data.
     * @param buf Buffer containing new data.
     * 
     * The new data is compared with the old data to determine the change mask.
     * The new data then replaces the old. Finally the change mask is returned.
     */
    uint64_t updateU(std::span<std::byte const> buf) {
        return update(buf, rxu_);
    }

    void switchPage(lpc865::SpiQueue &spi, uint8_t page) {
        page_ = std::byte(page);
        rdwr(spi, 0x7F, { &page_, 1 });
    }

    void writeRegs(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x01, regs_);
    }

    void writeCS(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x00, txcs_);
    }

    void writeU(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x40, txu_);
    }

    void readRegs(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x81, regs_);
    }

    void readTxStatus(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x8A, std::span(regs_).subspan(9,1));
    }

    void readRatio(lpc865::SpiQueue &spi) {
        rdwr(spi, 0xB2, std::span(regs_).subspan(49,2));
    }

    void readRxStatus(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x92, std::span(regs_).subspan(17,4));
    }

    void readSubchannel(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x9F, std::span(regs_).subspan(30,14));
    }

    void readCS(lpc865::SpiQueue &spi) {
        rdwr(spi, 0x80, rxcs_);
    }

    void readU(lpc865::SpiQueue &spi) {
        rdwr(spi, 0xC0, rxu_);
    }

    std::byte *getPtr(uint8_t addr, std::byte &page);

private:
    static uint64_t update(std::span<std::byte const>, std::span<std::byte>);

    void rdwr(lpc865::SpiQueue &spi, uint8_t reg, std::span<std::byte> buf);

    lpc865::SpiQueue::Entry entry_;
    std::byte page_;                    //!< Page register at 0x7F
    std::array<std::byte, 51> regs_;    //!< Page 0 addresses 0x01..0x33
    std::array<std::byte, 48> rxcs_;    //!< Page 1 addresses 0x00..0x2F
    std::array<std::byte, 48> rxu_;     //!< Page 1 addresses 0x40..0x6F
    std::array<std::byte, 48> txcs_;    //!< Page 2 addresses 0x00..0x2F
    std::array<std::byte, 48> txu_;     //!< Page 2 addresses 0x40..0x6F
};

} //!@} namespace
