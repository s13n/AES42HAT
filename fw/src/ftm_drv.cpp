/** @file
 * support for the LPC8 FTM block.
 * @addtogroup LPC865_ftm
 * @ingroup LPC865
 * @{
 */
#include "ftm_drv.hpp"
#include "FTM.hpp"
#include "handler.hpp"


uint16_t lpc865::Ftm::getCount() {
    auto &hw = *in_.registers;
    return hw.CNT.get().COUNT;
}

void lpc865::Ftm::setMatch(unsigned ch, uint16_t value) {
    auto &hw = *in_.registers;
    hw.CONTROLS[ch].CV = CV{ .VAL = value };
}

uint16_t lpc865::Ftm::getCapture(unsigned ch) {
    auto &hw = *in_.registers;
    return hw.CONTROLS[ch].CV.get().VAL;
}

void lpc865::Ftm::setHandlers(Handler *overflow, Handler *reload) {
    auto &hw = *in_.registers;
    overflow_ = overflow;
    reload_ = reload;
    auto sc = hw.SC.get();
    sc.TOIE = overflow != nullptr;
    sc.RIE = reload != nullptr;
    hw.SC = sc;
}

void lpc865::Ftm::setModulusDelta(int16_t delta) {
    auto &hw = *in_.registers;
    hw.MOD = uint16_t(mod_ + delta);
}

lpc865::Ftm::Ftm(FTM::Integration const &in, Parameters const &par)
    : mod_{par.mod}
    , in_{in}
    , overflow_{nullptr}
    , reload_{nullptr}
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
    hw.MOD = mod_;
    hw.HCR = par.hcyc;
    hw.PWMLOAD = PWMLOAD{ .HCSEL = par.hcyc != 0 };
    SC sc{ .PS=par.ps, .CLKS=par.clks, .CPWMS=par.updn };
    uint32_t oinit{};
    uint32_t pol{};
    uint32_t comb{};
    uint32_t pwmen{};
    auto oiMask = FIELDMASK(OUTINIT, CH0OI);
    auto polMask = FIELDMASK(POL, POL0);
    auto pwmenMask = FIELDMASK(SC, PWMEN0);
    auto combineMask = FIELDMASK(COMBINE, COMBINE0);
    auto compMask = FIELDMASK(COMBINE, COMP0);
    for (unsigned i = 0; i <= in_.max_channel; ++i, oiMask<<=1, polMask<<=1, pwmenMask<<=1, combineMask<<=1, compMask<<=1) {
        auto ch = par.ch[i];
        auto csc = cscTbl[ch.mode];
        csc.TRIGMODE = ch.trig;
        csc.CHIE = ch.intr;
        hw.CONTROLS[i].CSC = csc;
        hw.CONTROLS[i].CV = 0xFFFF;
        oinit |= ch.inv ? oiMask : 0;
        pol |= ch.inv ? polMask : 0;
        pwmen |= ch.mode >= pwmCombine ? pwmenMask : 0;
        if ((i & 1) == 0 && ch.mode == pwmCombine) {
            comb |= combineMask;
            comb |= compMask;
        }
    }
    hw.SC = std::bit_cast<uint32_t>(sc) | pwmen;
    hw.OUTINIT = oinit;
    hw.POL = pol;
    hw.MODE = MODE{ .FTMEN = 1, .INIT=1, .WPDIS=WPDIS_disabled };
    insert(in_.exFTM);
}

void lpc865::Ftm::isr() {
    auto &hw = *in_.registers;
    auto sc = hw.SC.get();
    if (overflow_ && sc.TOF) {
        sc.TOF = 0;
        overflow_->post();
    }
    if (reload_ && sc.RF) {
        sc.RF = 0;
        reload_->post();
    }
    hw.SC = sc;
}

/** @}*/
