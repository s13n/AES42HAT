/** @file
 * driver for the LPC8 I2C block in target mode.
 * 
 * @addtogroup LPC865_i2c
 * @ingroup LPC865
 * @{
 */

#pragma once

#include "nvic_drv.hpp"
#include "I2C.hpp"
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <span>


namespace lpc865 {

struct Event;
class DmaBase;

/** I2C Target Driver.
 */
class I2cTarget : public arm::Interrupt {
public:
    /** Callback representing one target address.
     *
     * Those callbacks are called in interrupt context.
     */
    class Callback {
        Callback(Callback &&) =delete;
    protected:
        ~Callback() =default;
        Callback() =default;
    public:
        virtual bool select(uint8_t) const =0;
        virtual void deselect() const =0;
        virtual uint8_t getTxByte() =0;
        virtual void putRxByte(uint8_t) =0;
    };

    /** Operational parameters. */
    struct Parameters {
        uint32_t dis0:1;    //!< Disable slave address 0
        uint32_t addr0:7;   //!< Slave address 0
        uint32_t dis1:1;    //!< Disable slave address 1
        uint32_t addr1:7;   //!< Slave address 1
        uint32_t dis2:1;    //!< Disable slave address 2
        uint32_t addr2:7;   //!< Slave address 2
        uint32_t dis3:1;    //!< Disable slave address 3
        uint32_t addr3:7;   //!< Slave address 3
        uint32_t qmode:1;   //!< Slave address 0 extension mode (0: mask, 1: range)
        uint32_t qual0:7;   //!< Slave address 0 extension (mask or range end)
        std::initializer_list<Callback*> callbacks;
    };

    void isr() override;

    I2cTarget(I2C::Integration const &in, Parameters const &par);
    ~I2cTarget() =default;

private:
    uint8_t target_;                        //!< Currently active target address / RW
    Callback *selected_;                    //!< The callback of the currently selected target
    I2C::Integration const &in_;            //!< Integration values
    Parameters const &par_;
};

} // namespace

//!@}
