/** @file
 * LPC865 dma driver.
 * @addtogroup LPC865_dma
 * @ingroup LPC865
 * @{
 */
#include "dma_drv.hpp"

constexpr lpc865::DmaBase::Channel *channels(lpc865::DmaBase *base) {
    return static_cast<lpc865::DmaBase::Channel *>(static_cast<void *>(base + 1));
}

constexpr size_t index(lpc865::DmaBase::Channel *chan) {
    return chan - channels(&chan->dma_);
}

lpc865::DmaBase::Channel::Channel(DmaBase &dma)
    : dma_{dma}
{
}

inline bool lpc865::DmaBase::Channel::init(SmartDMA::CFG cfg, SmartDMA::XFERCFG xfer, intptr_t addr) {
    auto chan = index(this);
    auto &hw = *dma_.in_.registers;
    assert(!(hw.ENABLESET0.val() & 1<<chan));   // channel not enabled

    auto &ch = hw.CHANNEL[chan];
    ch.CFG.set(cfg);
    auto &desc = dma_.desc_[chan];
    desc.xfer = xfer;
    if (ch.XFERCFG.get().SRCINC == 0)
        desc.src = addr;
    else
        desc.dst = addr;

    return true;
}

bool lpc865::DmaBase::Channel::start(BufferList buffers) {
    assert(buffers.size()-1 < dma_.desc_.size() / (dma_.in_.max_channel + 1));
    auto chan = index(this);
    auto &hw = *dma_.in_.registers;
    SmartDMA::XFERCFG xfer;
    for (size_t i = 0; i < buffers.size(); ++i) {
        auto di = i * (dma_.in_.max_channel + 1) + chan;
        if (di >= dma_.desc_.size())
            return false;
        auto &desc = dma_.desc_[di];
        if (i == 0) {
            xfer = desc.xfer;
        } else {
            desc.xfer = xfer;
            auto &prev = dma_.desc_[di - (dma_.in_.max_channel + 1)];
            prev.xfer = xfer;
            prev.xfer.RELOAD = 1;
            prev.link = reinterpret_cast<uintptr_t>(&desc);
        }
        auto &addr = hw.CHANNEL[chan].XFERCFG.get().SRCINC != 0 ? desc.src : desc.dst;
        addr = reinterpret_cast<uintptr_t>(&buffers[i].back());
    }
    hw.CHANNEL[chan].XFERCFG.set(xfer);
    hw.ENABLESET0.set(1U<<chan);
    return true;
}

void lpc865::DmaBase::Channel::stop() {
    auto chan = index(this);
    auto &hw = *dma_.in_.registers;
    hw.ABORT0.set(1U<<chan);
}

bool lpc865::DmaBase::Channel::busy() {
    auto chan = index(this);
    auto &hw = *dma_.in_.registers;
    return hw.BUSY0.val() & 1U<<chan;
}

bool lpc865::DmaBase::Channel::completed() {
    auto chan = index(this);
    auto &hw = *dma_.in_.registers;
    return !(hw.ACTIVE0.val() & 1U<<chan);
}

lpc865::DmaBase::~DmaBase() {
    auto &hw = *in_.registers;
    hw.CTRL.set(0);     // disable
}

lpc865::DmaBase::DmaBase(SmartDMA::Integration const &in, void *mem, size_t size)
    : in_{in}
    , desc_{static_cast<Channel::Descriptor *>(mem), size / sizeof(Channel::Descriptor)}
{
    auto &hw = *in_.registers;
    hw.CTRL.set(0);
    hw.ENABLECLR0.set(hw.ENABLESET0.val());
    hw.INTENCLR0.set(hw.INTENSET0.val());
    std::fill(desc_.begin(), desc_.end(), Channel::Descriptor{});
    auto srambase = reinterpret_cast<uint32_t>(mem);
    assert(srambase % 512 == 0);
    hw.SRAMBASE.set(srambase);
}

void lpc865::DmaBase::isr() {
}

auto lpc865::DmaBase::setup(unsigned chan, SmartDMA::CFG cfg, SmartDMA::XFERCFG xfer, intptr_t addr) -> Channel* {
    Channel *ch = channels(this) + chan;
    if (ch && ch->init(cfg, xfer, addr))
        return ch;
    return nullptr;
}

/** @}*/
