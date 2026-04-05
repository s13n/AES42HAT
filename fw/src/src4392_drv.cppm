/** @file
 * SRC4392 driver.
 * @addtogroup SRC4392
 * @{
 */

module;
#include <array>
#include <cstdint>
#include <span>
export module src4392_drv;
import spi_queue;
import handler;
import SRC4392;

export namespace src4392 {

/** SRC4392 driver class.
 */
class Src4392 {
public:
    Src4392(SRC4392::Intgr const &in, Handler *hdl);

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

    void switchPage(lpc865::SpiQueue &spiq, uint8_t page) {
        page_ = std::byte(page);
        rdwr(spiq, { &page_, 1 }, 0x7F);
    }

    void writeRegs(lpc865::SpiQueue &spiq) {
        rdwr(spiq, regs_, 0x01);
    }

    void writeCS(lpc865::SpiQueue &spiq) {
        rdwr(spiq, txcs_, 0x00);
    }

    void writeU(lpc865::SpiQueue &spiq) {
        rdwr(spiq, txu_, 0x40);
    }

    void readRegs(lpc865::SpiQueue &spiq) {
        rdwr(spiq, regs_, 0x81);
    }

    void readTxStatus(lpc865::SpiQueue &spiq) {
        rdwr(spiq, std::span(regs_).subspan(9,1), 0x8A);
    }

    void readRatio(lpc865::SpiQueue &spiq) {
        rdwr(spiq, std::span(regs_).subspan(49,2), 0xB2);
    }

    void readRxStatus(lpc865::SpiQueue &spiq) {
        rdwr(spiq, std::span(regs_).subspan(17,4), 0x92);
    }

    void readSubchannel(lpc865::SpiQueue &spiq) {
        rdwr(spiq, std::span(regs_).subspan(30,14), 0x9F);
    }

    void readCS(lpc865::SpiQueue &spiq) {
        rdwr(spiq, rxcs_, 0x80);
    }

    void readU(lpc865::SpiQueue &spiq) {
        rdwr(spiq, rxu_, 0xC0);
    }

    std::byte *getPtr(uint8_t addr, std::byte &page);

private:
    static uint64_t update(std::span<std::byte const>, std::span<std::byte>);

    void rdwr(lpc865::SpiQueue &, std::span<std::byte>, uint8_t);

    lpc865::SpiQueue::Entry entry_;
    std::byte page_;                    //!< Page register at 0x7F
    std::array<std::byte, 51> regs_;    //!< Page 0 addresses 0x01..0x33
    std::array<std::byte, 48> rxcs_;    //!< Page 1 addresses 0x00..0x2F
    std::array<std::byte, 48> rxu_;     //!< Page 1 addresses 0x40..0x6F
    std::array<std::byte, 48> txcs_;    //!< Page 2 addresses 0x00..0x2F
    std::array<std::byte, 48> txu_;     //!< Page 2 addresses 0x40..0x6F
};

} //!@} namespace
