/** @file
 * support for the LPC8 FTM block.
 * @addtogroup LPC865_ftm
 * @ingroup LPC865
 * @{
 */
#include "ftm_drv.hpp"


lpc865::Ftm::Ftm(FTM::Integration const &in, Parameters const &par)
    : in_{in}
{
    auto &hw = *in_.registers;
    hw.SC = SC{ .PS=par.ps, .CLKS=par.clks };
}

/** @}*/
 