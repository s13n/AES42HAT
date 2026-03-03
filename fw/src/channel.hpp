/** @file
 * object representing a channel with one SRC4392
 * 
 * @addtogroup Channel
 * @ingroup AES42HAT
 * @{
 */

#pragma once

#include "i2c_tgt_drv.hpp"
#include "nvic_drv.hpp"
#include "coroutine.hpp"
#include "handler.hpp"
#include "src4392_drv.hpp"
#include "SRC4392.hpp"

namespace lpc865 {
    class Ftm;
    class Pint;
    class SpiQueue;
}

/** Object representing an AES42 channel.
 * 
 * The channel object represents an SRC4392 with its associated circuitry.
 * It processes the interrupt coming from the interrupt pin of the SRC4392,
 * which is received as a pin interrupt, but also triggers a capture event
 * on FTM0. The capture event doesn't cause an interrupt, even though it
 * could, because pin interrupts are more direct and efficient.
 * 
 * In normal operation, the interrupt is set up to fire once for each block
 * received on the DIR of the channel. This signals that
 * - A phase measurement was taken by FTM0
 * - A new block of channel status and user data has been received
 * 
 * This interrupt triggers a sequence of SPI read transfers to copy the
 * receive status, the CS data and the U-bit data to the buffers in src_
 * 
 * The channel is also attached to the I2C target interface, so that the
 * host can set and get register settings of the SRC4392. The host has
 * the impression of talking directly to an SRC4392 in this way.
 */
class Channel : public lpc865::I2cTarget::Callback, public arm::Interrupt, public Handler {
public:
    struct Integration {
        src4392::SRC4392::Intgr in;
        uint16_t irq:3;     //!< PINT channel for this channel
        uint16_t tch:3;     //!< Timer channel associated with this channel
        uint16_t rch:3;     //!< Reference channel in timer to compare timestamps with
        std::initializer_list<std::byte> init;  //!< Initialization data for SRC registers
    };

    bool select(uint8_t) override;
    void deselect() override;
    uint8_t getTxByte() override;
    void putRxByte(uint8_t) override;

    void isr() override;

    /** Handles the receive side block event. */
    void act() override;

    /** Write the cached register data to the SRC chip */
    void updateSrcCtrl() {
        pg0wb_ = true;
        post();
    }

    /** Handles the transmit side block event. */
    void handleTxBlock() {
        pg2wb_ = true;
        post();
    }

    Channel(Integration const &in, lpc865::SpiQueue &spiq, lpc865::Ftm &ftm, lpc865::Pint &pint);

private:
    uint8_t addr_;              //!< Current register address byte (MSB = INC bit) in I2C access
    bool expectReg_;            //!< True when expecting register address byte from I2C
    std::byte page_;            //!< Page in access from the I2C side
    Coroutine<int8_t> coro_;    //!< Coroutine to read the RX status, CS and U data
    bool volatile pg0wb_;       //!< Page 0 (Control registers) needs writing back to chip
    bool volatile pg2wb_;       //!< Page 2 (DIT CS&U data) needs writing back to chip
    bool volatile rstat_;       //!< Page 0 receive status registers need reading from chip
    bool volatile pg1rd_;       //!< Page 1 (DIR CS&U data) needs reading from the chip
    int16_t delta_;             //!< Timestamp difference relative to BLS pulse
    Integration const &in_;     //!< Channel integration data
    lpc865::SpiQueue &spiq_;    //!< SPI port driver to use for controlling the channel
    lpc865::Ftm &ftm_;          //!< Timer responsible for phase management
    lpc865::Pint &pint_;        //!< Pin interrupt driver
    src4392::Src4392 src_;      //!< SRC4392 register set cache
};
