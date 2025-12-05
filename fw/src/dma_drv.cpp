/** @file
 * LPC865 dma driver.
 * @addtogroup LPC865_dma
 * @ingroup LPC865
 * @{
 */
#include "dma_drv.hpp"
#include "SmartDMA.hpp"
#include "handler.hpp"
#include <bit>
#include <cassert>


bool lpc865::Dma::setup(Per per, uintptr_t addr, Handler *hdl) {
    if (per.chan > in_.max_channel)
        return false;
    auto &desc = par_.descs[per.chan];
    desc.xfer = std::bit_cast<uint32_t>(per);
    if (per.dest)
        desc.dst = addr;
    else
        desc.src = addr;
    desc.link = uintptr_t(hdl);
    return true;
}

bool lpc865::Dma::start(Mem mem, void *buf, size_t size) {
    if (mem.chan > in_.max_channel || !buf || size == 0)
        return false;
    auto &hw = *in_.registers;
    auto &desc = par_.descs[mem.chan];
    auto &chan = hw.CHANNEL[mem.chan];
    auto per = std::bit_cast<Per>(desc.xfer);
    if (per.dest)
        desc.src = reinterpret_cast<uintptr_t>(buf) + size;
    else
        desc.dst = reinterpret_cast<uintptr_t>(buf) + size;
    uint32_t mask = 1u << mem.chan;
    hw.ENABLECLR0 = mask;
    chan.CFG = CFG{
        .PERIPHREQEN=1, .HWTRIGEN=per.hwtrig, .TRIGPOL=per.trigpol,
        .TRIGTYPE=per.trigtype, .TRIGBURST=per.trigburst, .BURSTPOWER=mem.burstpower,
        .SRCBURSTWRAP=per.dest?mem.burstwrap:0u, .DSTBURSTWRAP=per.dest?0u:mem.burstwrap,
        .CHPRIORITY=mem.prio
    };
    chan.XFERCFG = XFERCFG{
        .CFGVALID=1, .RELOAD=0, .SWTRIG=1, .CLRTRIG=1, .SETINTA=mem.setintA, .SETINTB=mem.setintB,
        .WIDTH=per.width, .SRCINC=per.dest?mem.inc:0u, .DSTINC=per.dest?0u:mem.inc,
        .XFERCOUNT=(size << per.width) - 1
    };
    if (mem.setintA || mem.setintB)
        hw.INTENSET0 = mask;
    else
        hw.INTENCLR0 = mask;
    hw.INTA0 = mask;
    hw.INTB0 = mask;
    hw.SETVALID0 = mask;
    hw.ENABLESET0 = mask;
    return true;
}

lpc865::Dma::~Dma() {
    auto &hw = *in_.registers;
    hw.CTRL.set(0);     // disable
}

lpc865::Dma::Dma(SmartDMA::Integration const &in, Parameters const &par)
    : in_{in}
    , par_{par}
{
    auto &hw = *in_.registers;
    hw.CTRL.set(0);
    hw.ENABLECLR0.set(hw.ENABLESET0.val());
    hw.INTENCLR0.set(hw.INTENSET0.val());
    auto srambase = reinterpret_cast<uint32_t>(par_.descs);
    assert(srambase % 512 == 0);
    hw.SRAMBASE.set(srambase);
    hw.CTRL = CTRL{ .ENABLE=1 };
}

void lpc865::Dma::isr() {
    auto &hw = *in_.registers;
    auto *descs = par_.descs;
    auto inta = hw.INTA0.get().IA;
    hw.INTA0 = inta;
    while (inta) {
        auto ch = std::countr_zero(inta);
        uint32_t mask = 1u << ch;
        inta &= ~mask;
        if (auto hdl = reinterpret_cast<Handler*>(descs[ch].link))
            hdl->post();
    }
    auto intb = hw.INTB0.get().IB;
    hw.INTB0 = intb;
    while (intb) {
        auto ch = std::countr_zero(intb);
        uint32_t mask = 1u << ch;
        intb &= ~mask;
        if (auto hdl = reinterpret_cast<Handler*>(descs[ch].link))
            hdl->post();
    }
}

/** @}*/
