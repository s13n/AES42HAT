/** @file
 * LPC865 PINT driver
 * 
 * @addtogroup LPC865_pint
 * @ingroup LPC865
 * @{
 */

#pragma once

#include <cstdint>

namespace arm {
    class Interrupt;
}

namespace lpc865 {

inline namespace PINT {
    struct Integration;
}

/** Pin interrupt driver.
 */
class Pint {
public:
    /** Attach and enable pin interrupt.
     * @param mode 0: none, 1: rising edge, 2: falling edge, 3: both edges, 4: low level, 5: high level
     */
    void attach(unsigned num, uint8_t mode, arm::Interrupt &intr);
    void enable(unsigned num, uint8_t mode);
    void disable(unsigned num);

    Pint(PINT::Integration const &in);
    ~Pint() =default;

private:
    PINT::Integration const &in_;
};

} //!@}
