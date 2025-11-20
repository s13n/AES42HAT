/** @file
 * Generic handler interface
 * 
 * @addtogroup Channel
 * @ingroup AES42HAT
 * @{
 */

#include "handler.hpp"
#include "nvic_drv.hpp"

static Handler* anchor = nullptr;

bool Handler::post() {
    bool result = false;
    arm::disable_irq();
    if(!next_) {      // post only if not already posted
        if(anchor) {
            // insert new Handler between anchor and anchor->next_
            next_ = anchor->next_;
            anchor->next_ = this;
        } else {
            next_ = this;  // form a ring of one single Handler
        }
        anchor = this;  // make anchor point to newest Handler
        result = true;
    }
    arm::enable_irq();
    return result;
}

inline Handler *Handler::unque() {
    Handler *res = nullptr;
    arm::disable_irq();
    if(anchor) {
        res = anchor->next_;
        if(res == res->next_)
            anchor = nullptr;
        else
            anchor->next_ = res->next_;
    }
    arm::enable_irq();
    return res;
}

size_t Handler::poll_one() {
    if (auto *node = unque()) {
        node->next_ = nullptr;
        node->act();
        return 1;
    }
    return 0;
}

size_t Handler::poll() {
    size_t res = 0;
    while(poll_one())
        ++res;
    return res;
}

void Handler::run() {
    while (true)
        if (poll_one() == 0)
            arm::wfe();
}
