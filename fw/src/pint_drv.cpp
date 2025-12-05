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

void lpc865::Pint::attach(unsigned num, uint8_t mode, arm::Interrupt &intr) {
    intr.insert(getEx(num, in_));
    enable(num, mode);
}

void lpc865::Pint::enable(unsigned num, uint8_t mode) {
    auto &hw = *in_.registers;
    uint32_t mask = 1u << num;
    switch (mode) {
    case 0:
        hw.ISEL = hw.ISEL.val() & ~mask;
        hw.CIENR = mask;
        hw.CIENF = mask;
        return;
    case 1:
        hw.ISEL = hw.ISEL.val() & ~mask;
        hw.SIENR = mask;
        hw.CIENF = mask;
        return;
    case 2:
        hw.ISEL = hw.ISEL.val() & ~mask;
        hw.CIENR = mask;
        hw.SIENF = mask;
        return;
    case 3:
        hw.ISEL = hw.ISEL.val() & ~mask;
        hw.SIENR = mask;
        hw.SIENF = mask;
        return;
    case 4:
        hw.ISEL = hw.ISEL.val() | mask;
        hw.SIENR = mask;
        hw.CIENF = mask;
        return;
    case 5:
        hw.ISEL = hw.ISEL.val() | mask;
        hw.SIENR = mask;
        hw.SIENF = mask;
        return;
    }
}

void lpc865::Pint::disable(unsigned num) {
    enable(num, 0);
}

lpc865::Pint::Pint(PINT::Integration const &in)
    : in_{in}
{
    auto &hw = *in_.registers;
    hw.ISEL.set(0x00);
    hw.IENR.set(0x00);
    hw.IENF.set(0x00);
    hw.IST.set(0xFF);
}

/** @}*/
