/** @file
 * LPC865 PINT driver
 * @addtogroup LPC865_pint
 * @ingroup LPC865
 * @{
 */
#include "pint_drv.hpp"
#include "nvic_drv.hpp"
#include "PINT.hpp"


constexpr Exception getEx(unsigned num, lpc865::Integration const &in) {
    switch (num) {
    case 0: return in.exINT0;
    case 1: return in.exINT1;
    case 2: return in.exINT2;
    case 3: return in.exINT3;
    case 4: return in.exINT4;
    case 5: return in.exINT5;
    case 6: return in.exINT6;
    case 7: return in.exINT7;
    }
    return Exception{};
}

void lpc865::Pint::attach(unsigned num, arm::Interrupt &intr) {
    intr.insert(getEx(num, in_));
    enable(num);
}

void lpc865::Pint::enable(unsigned num) {
    in_.registers->SIENR.set(1u << num);
}

void lpc865::Pint::disable(unsigned num) {
    in_.registers->CIENR.set(1u << num);
}

lpc865::Pint::Pint(PINT::Integration const &in)
    : in_{in}
{
    auto &hw = *in_.registers;
    hw.ISEL.set(0xFF);
}

/** @}*/
