/** @file
 * object representing a channel with one SRC4392
 * @addtogroup Channel
 * @ingroup AES42HAT
 * @{
 */
#include "channel.hpp"
#include "pint_drv.hpp"
#include "ftm_drv.hpp"
#include <string_view>

extern void print(std::string_view);


bool Channel::select(uint8_t tgt) {
    print("S");
    if ((tgt >> 1) != in_.i2cAddr)
        return false;
    expectReg_ = !(tgt & 0x01);
    return true;
}

void Channel::deselect() {
    print("D\n");
    if (updateSrcPage_ & 0x01) {
        updateSrcCtrl();
        updateSrcPage_ &= ~0x01;
    }
}

uint8_t Channel::getTxByte() {
    print("T");
    std::byte *ptr = src_.getPtr(addr_ & 0x7F, page_);
    if (bool inc = addr_ & 0x80)
        addr_ = (addr_ + 1) | 0x80;
    return ptr ? uint8_t(*ptr) : 0;
}

void Channel::putRxByte(uint8_t val) {
    print("R");
    if (expectReg_) {
        addr_ = val;
        expectReg_ = false;
        return;
    }
    std::byte *ptr = src_.getPtr(addr_ & 0x7F, page_);
    if (bool inc = addr_ & 0x80)
        addr_ = (addr_ + 1) | 0x80;
    if (ptr) {
        *ptr = std::byte(val);
        updateSrcPage_ |= 1U << uint8_t(page_);
    }
}

void Channel::isr() {
    uint16_t ts1 = ftm_.getCount();
    uint16_t capt = ftm_.getCapture(2 + (in_.i2cAddr & 0x03));
    uint16_t ref = ftm_.getCapture(0);
    uint16_t ts2 = ftm_.getCount();
    delta_ = capt - ref;
    pint_.disable(in_.irq);
    post();
}

void Channel::act() {
    CORO_REENTER(coro_) {
        CORO_YIELD src_.readRxStatus(spiq_);
        CORO_YIELD src_.switchPage(spiq_, 0x01);
        CORO_YIELD src_.readCS(spiq_);
        CORO_YIELD src_.readU(spiq_);
        CORO_YIELD src_.switchPage(spiq_, 0x00);
        pint_.enable(in_.irq, 4);
    }
}

void Channel::updateSrcCtrl() {
    src_.writeRegs(spiq_);
}

void Channel::handleTxBlock() {
    src_.readTxStatus(spiq_);
}

Channel::Channel(Integration const &in, lpc865::SpiQueue &spiq, lpc865::Ftm &ftm, lpc865::Pint &pint)
    : addr_{0}
    , expectReg_{false}
    , page_{0}
    , updateSrcPage_{0}
    , in_{in}
    , spiq_{spiq}
    , ftm_{ftm}
    , pint_{pint}
    , src_{in.in, this}
{
    src_.updateRegs(in_.init);
    pint_.attach(in_.irq, 4, *this);
}
