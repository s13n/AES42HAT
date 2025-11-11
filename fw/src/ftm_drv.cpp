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
    static constexpr CSC cscTbl[] = {
        { .ELSA = 0, .ELSB = 0, .MSA = 0, .MSB = 0 },   // off
        { .ELSA = 1, .ELSB = 0, .MSA = 0, .MSB = 0 },   // capturePos
        { .ELSA = 0, .ELSB = 1, .MSA = 0, .MSB = 0 },   // captureNeg
        { .ELSA = 1, .ELSB = 1, .MSA = 0, .MSB = 0 },   // captureBoth
        { .ELSA = 0, .ELSB = 1, .MSA = 1, .MSB = 0 },   // pwmCombine
        { .ELSA = 1, .ELSB = 0, .MSA = 1, .MSB = 0 },   // compareToggle
        { .ELSA = 0, .ELSB = 1, .MSA = 0, .MSB = 1 },   // pwmPos
        { .ELSA = 1, .ELSB = 1, .MSA = 0, .MSB = 1 },   // pwmNeg
    };

    auto &hw = *in_.registers;
    hw.MODE = MODE{ .WPDIS=WPDIS_disabled };
    hw.MOD = par.mod;
    SC sc{ .PS=par.ps, .CLKS=par.clks, .CPWMS=par.updn };
    OUTINIT oinit{};
    COMBINE comb{};
    POL pol{};
    for (unsigned i = 0; i <= in_.max_channel; ++i) {
        switch (i) {
        case 0:
            hw.CONTROLS[0].CSC = cscTbl[par.ch0];
            hw.CONTROLS[0].CV = 0xFFFF;
            oinit.CH0OI = par.ch0inv;
            pol.POL0 = par.ch0inv;
            sc.PWMEN0 = par.ch0 >= pwmCombine;
            if (par.ch0 == pwmCombine) {
                comb.COMBINE0 = 1;
                comb.COMP0 = 1;
            }
            break;
        case 1:
            hw.CONTROLS[1].CSC = cscTbl[par.ch1];
            hw.CONTROLS[1].CV = 0xFFFF;
            oinit.CH1OI = par.ch1inv;
            pol.POL1 = par.ch1inv;
            sc.PWMEN1 = par.ch1 >= pwmCombine;
            break;
        case 2:
            hw.CONTROLS[2].CSC = cscTbl[par.ch2];
            hw.CONTROLS[2].CV = 0xFFFF;
            oinit.CH2OI = par.ch2inv;
            pol.POL2 = par.ch2inv;
            sc.PWMEN2 = par.ch2 >= pwmCombine;
            if (par.ch2 == pwmCombine) {
                comb.COMBINE1 = 1;
                comb.COMP1 = 1;
            }
            break;
        case 3:
            hw.CONTROLS[3].CSC = cscTbl[par.ch3];
            hw.CONTROLS[3].CV = 0xFFFF;
            oinit.CH3OI = par.ch3inv;
            pol.POL3 = par.ch3inv;
            sc.PWMEN3 = par.ch3 >= pwmCombine;
            break;
        case 4:
            hw.CONTROLS[4].CSC = cscTbl[par.ch4];
            hw.CONTROLS[4].CV = 0xFFFF;
            oinit.CH4OI = par.ch4inv;
            pol.POL4 = par.ch4inv;
            sc.PWMEN4 = par.ch4 >= pwmCombine;
            if (par.ch4 == pwmCombine) {
                comb.COMBINE2 = 1;
                comb.COMP2 = 1;
            }
            break;
        case 5:
            hw.CONTROLS[5].CSC = cscTbl[par.ch5];
            hw.CONTROLS[5].CV = 0xFFFF;
            oinit.CH5OI = par.ch5inv;
            pol.POL5 = par.ch5inv;
            sc.PWMEN5 = par.ch5 >= pwmCombine;
            break;
        case 6:
            hw.CONTROLS[6].CSC = cscTbl[par.ch6];
            hw.CONTROLS[6].CV = 0xFFFF;
            oinit.CH6OI = par.ch6inv;
            pol.POL6 = par.ch6inv;
//            sc.PWMEN6 = par.ch6 >= pwmCombine;
            if (par.ch6 == pwmCombine) {
                comb.COMBINE3 = 1;
                comb.COMP3 = 1;
            }
            break;
        case 7:
            hw.CONTROLS[7].CSC = cscTbl[par.ch7];
            hw.CONTROLS[7].CV = 0xFFFF;
            oinit.CH7OI = par.ch7inv;
            pol.POL7 = par.ch7inv;
//            sc.PWMEN7 = par.ch7 >= pwmCombine;
            break;
        }
    }
    hw.SC = sc;
    hw.OUTINIT = oinit;
    hw.POL = pol;
    hw.MODE = MODE{ .INIT=1, .WPDIS=WPDIS_disabled };
}

/** @}*/
 