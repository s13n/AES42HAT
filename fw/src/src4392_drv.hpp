/** @file
 * SRC4392 driver.
 * @addtogroup SRC4392
 * @{
 */
#pragma once
#include "SRC4392.hpp"
#include <array>
#include <span>

namespace lpc865 {
    class Spi;
}

namespace src4392 { 

/** SRC4392 driver class.
 */     
class Src4392 { 
    Src4392(Src4392 &&) = delete;
public:

    Src4392(Integration const &in);

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

    void writeRegs(lpc865::Spi &spi);

//private:
    static uint64_t update(std::span<std::byte const>, std::span<std::byte>);

    Integration const &in_;
    std::byte page_;                    //!< Page register at 0x7F
    std::array<std::byte, 51> regs_;    //!< Page 0 addresses 0x01..0x33
    std::array<std::byte, 48> rxcs_;    //!< Page 1 addresses 0x00..0x2F
    std::array<std::byte, 48> rxu_;     //!< Page 1 addresses 0x40..0x6F
    std::array<std::byte, 48> txcs_;    //!< Page 2 addresses 0x00..0x2F
    std::array<std::byte, 48> txu_;     //!< Page 0 addresses 0x40..0x6F
};

} //!@} namespace
