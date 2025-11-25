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
#include "handler.hpp"
#include "src4392_drv.hpp"

namespace lpc865 {
    class Ftm;
    class Pint;
    class Spi;
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
 * The channel is also attached to the I2C target interface, so that the
 * host can set and get register settings of the SRC4392. The host has
 * the impression of talking directly to an SRC4392 in this way.
 */
class Channel : public lpc865::I2cTarget::Callback, public arm::Interrupt, public Handler {
public:
    bool select(uint8_t) override;
    void deselect() override;
    uint8_t getTxByte() override;
    void putRxByte(uint8_t) override;

    void isr() override;

    /** Handles the receive side block event. */
    void act() override;

    void updateSrcCtrl();

    /** Handles the transmit side block event. */
    void handleTxBlock();

    Channel(lpc865::Spi &spi, lpc865::Ftm &ftm, lpc865::Pint &pint, uint8_t ch, uint8_t irq);

private:
    uint8_t myaddr_;        //!< I2C target address of this channel
    uint8_t addr_;          //!< Current register address byte (MSB = INC bit)
    bool expectReg_;        //!< True when expecting register address byte
    std::byte page_;        //!< Page in access from the I2C side
    uint8_t updateSrcPage_; //!< Pages in SRC chip that needs updating (1 bit for each page)
    uint8_t irq_;
    int16_t delta_;
    uint64_t updatesRegs_;
    uint64_t updatesCS_;
    uint64_t updatesU_;
    lpc865::Spi &spi_;      //!< SPI port driver to use for controlling the channel
    lpc865::Ftm &ftm_;      //!< Timer responsible for phase management
    lpc865::Pint &pint_;
    src4392::Src4392 src_;  //!< SRC4392 register set cache
};
