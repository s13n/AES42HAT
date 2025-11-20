/** @file
 * ARM NVIC driver
 * 
 * @addtogroup ARM_NVIC
 * @ingroup ARM
 * @{
 */

#pragma once

#include "registers.hpp"
#include <cstddef>
#include <cstdint>

namespace arm {

__attribute__((always_inline)) inline void enable_irq() {
    asm volatile ("cpsie i" : : : "memory");
}

__attribute__((always_inline)) inline void disable_irq() {
    asm volatile ("cpsid i" : : : "memory");
}

__attribute__((always_inline)) inline void wfe(void) {
    asm volatile ("wfe");
}


class Interrupt {
    Interrupt(Interrupt &&) =delete;
protected:
    ~Interrupt() =default;
    Interrupt() =default;
public:
    typedef void (VectorTableEntry)();

    virtual void isr() =0;
    
    void enable(Exception);
    void disable(Exception);
    void insert(Exception);
    static void defaultISR();

private:
    static Interrupt *get_head(Exception n);

    Interrupt *link_;
};

}
