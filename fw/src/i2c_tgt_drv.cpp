/** @file
 * driver for the LPC8 I2C block in target mode.
 * @addtogroup LPC865_i2c
 * @ingroup LPC865
 * @{
 */
#include "i2c_tgt_drv.hpp"
#include <bit>


void lpc865::I2cTarget::isr() {
    auto &hw = *in_.registers;
    auto stat = hw.STAT.get();
    if (stat.SLVDESEL) {
        if (selected_)
            selected_->deselect();
        hw.STAT = STAT{ .SLVDESEL = 1 };
    }
    if (stat.SLVSEL) {
        switch (stat.SLVSTATE) {
        case SLAVE_ADDRESS:
            target_ = hw.SLVDAT.get().DATA;
            selected_ = nullptr;
            for (auto callback : par_.callbacks) {
                if (callback->select(target_)) {
                    selected_ = callback;
                    break;
                }
            }
            if (selected_) {
                if (target_ & 1)    // read --> target needs to send data
                    hw.SLVDAT.set(selected_->getTxByte());
                hw.SLVCTL = SLVCTL{ .SLVCONTINUE = 1 };
            } else {
                target_ = 0xFF;
            }
            break;
        case SLAVE_RECEIVE:
            if (uint8_t d = hw.SLVDAT.get().DATA; selected_)
                selected_->putRxByte(d);
            hw.SLVCTL = SLVCTL{ .SLVCONTINUE = 1 };
            break;
        case SLAVE_TRANSMIT:
            if (selected_)
                hw.SLVDAT.set(selected_->getTxByte());
            hw.SLVCTL = SLVCTL{ .SLVCONTINUE = 1 };
            break;
        default:
            break;
        }
    } else {
        target_ = 0xFF;
        selected_ = nullptr;
    }
}

lpc865::I2cTarget::I2cTarget(I2C::Integration const &in, Parameters const &par)
    : target_{0xFF}
    , selected_{nullptr}
    , in_{in}
    , par_{par}
{
    // Check if the callback list matches the slave addresses
    size_t n = par.qmode ? par.qual0 - par.addr0 + 1 : 1U << std::popcount(par.qual0);
    n = (par.dis0 ? 0 : n) + !par.dis1 + !par.dis1 + !par.dis1;
    if (par.callbacks.size() != n)
        return;
    
    insert(in_.exI2C);

    auto &hw = *in_.registers;
    hw.SLVADR[0] = SLVADR{ .SADISABLE = par.dis0, .SLVADR = par.addr0 };
    hw.SLVADR[1] = SLVADR{ .SADISABLE = par.dis1, .SLVADR = par.addr1 };
    hw.SLVADR[2] = SLVADR{ .SADISABLE = par.dis2, .SLVADR = par.addr2 };
    hw.SLVADR[3] = SLVADR{ .SADISABLE = par.dis3, .SLVADR = par.addr3 };
    hw.SLVQUAL0 = SLVQUAL0{ .QUALMODE0 = par.qmode, .SLVQUAL0 = par.qual0 };
    hw.INTENSET = INTENSET{ .SLVPENDINGEN = 1, .SLVDESELEN = 1 };
    hw.CFG = CFG{ .SLVEN = 1 };
}

/** @}*/
