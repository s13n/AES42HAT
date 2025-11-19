/** @file
 * Generic handler interface
 * 
 * @addtogroup Channel
 * @ingroup AES42HAT
 * @{
 */

#include "handler.hpp"

static Handler* anchor = nullptr;

__attribute__((always_inline)) inline void enable_irq() {
    asm volatile ("cpsie i" : : : "memory");
}

__attribute__((always_inline)) inline void disable_irq() {
    asm volatile ("cpsid i" : : : "memory");
}

__attribute__((always_inline)) inline void wfe(void) {
  asm volatile ("wfe");
}

bool Handler::post() {
    bool result = false;
    disable_irq();
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
    enable_irq();
    return result;
}

inline Handler *Handler::unque() {
    Handler *res = nullptr;
    disable_irq();
    if(anchor) {
        res = anchor->next_;
        if(res == res->next_)
            anchor = nullptr;
        else
            anchor->next_ = res->next_;
    }
    enable_irq();
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
            wfe();
}
