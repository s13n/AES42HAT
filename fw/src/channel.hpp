/** @file
 * object representing a channel with one SRC4392
 * 
 * @addtogroup Channel
 * @ingroup AES42HAT
 * @{
 */

#pragma once

#include "i2c_tgt_drv.hpp"
#include "src4392_drv.hpp"

namespace lpc865 {
    class Spi;
}

class Channel : public lpc865::I2cTarget::Callback {
public:
    bool select(uint8_t) override;
    void deselect() override;
    uint8_t getTxByte() override;
    void putRxByte(uint8_t) override;

    void updateSrcCtrl();
    
    Channel(unsigned ch, lpc865::Spi &spi);

private:
    uint8_t myaddr_;        //!< I2C target address of this channel
    uint8_t addr_;          //!< Current register address byte (MSB = INC bit)
    bool expectReg_;//!< True when expecting register address byte
    uint8_t updateSrcPage_; //!< Pages in SRC chip that needs updating (1 bit for each page)
    lpc865::Spi &spi_;
    src4392::Src4392 src_;
};
