/** @file
 * LPC865 PINT driver
 * 
 * @addtogroup LPC865_pint
 * @ingroup LPC865
 * @{
 */

#pragma once

#include <cstdint>

import nvic_drv;
import PINT;

namespace lpc865 {

/** Pin interrupt driver.
 */
class Pint {
public:
    /** Attach and enable pin interrupt.
     * @param mode 0: none, 1: rising edge, 2: falling edge, 3: both edges, 4: low level, 5: high level
     */
    void attach(unsigned num, uint8_t mode, arm::Interrupt &intr);

    void enable(unsigned num, uint8_t mode);

    void disable(unsigned num) {
        enable(num, 0);
    }

    Pint(PINT::Intgr const &in);

private:
    PINT::Intgr const &in_;
};

} //!@}
