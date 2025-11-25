/** @file
 * LPC865 PINT driver
 * 
 * @addtogroup LPC865_pint
 * @ingroup LPC865
 * @{
 */

#pragma once

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
    void attach(unsigned num, arm::Interrupt &intr);
    void enable(unsigned num);
    void disable(unsigned num);

    Pint(PINT::Integration const &in);
    ~Pint() =default;

private:
    PINT::Integration const &in_;
};

} //!@}
