/** @file
 * ARM NVIC driver
 * 
 * @addtogroup ARM_NVIC
 * @ingroup ARM
 * @{
 */

#include "nvic_drv.hpp"
#include "chip.hpp"
#include <array>
extern "C" {
#   include "newlib_def.h"
}

using namespace lpc865;

static std::array<arm::Interrupt *, i_NVIC.interrupts + 16> interrupt_table = {};

/** get a pointer to the ring head for the given exception number
 * @param n Exception number
 * @return Pointer to ring head, or nullptr if exception number was invalid
 */
inline arm::Interrupt *arm::Interrupt::get_head(Exception n) {
    // We use some ugly casts to simulate a list head that looks like an
    // object of type InterruptHandler, even though it is only a pointer.
    // This simplifies the list insertion, deletion and traversal operations.
    if(n >= interrupt_table.size() || n == 0)
        return nullptr;
    auto offset = (char*)&(interrupt_table[n]->link_) - (char*)(interrupt_table[n]);
    auto object = (char*)(&interrupt_table[n]) - offset;
    return reinterpret_cast<arm::Interrupt *>(object);
}

void arm::Interrupt::defaultISR() {
    unsigned exnum;                                                                     
    void *stack;                                                                        
    asm volatile (" mov %0, LR " : "=r" (exnum));       // get exception return address  
    if(exnum & 4)                                                                       
        asm volatile (" mrs %0, PSP " : "=r" (stack));                                  
    else                                                                                
        asm volatile (" mrs %0, MSP " : "=r" (stack));                                  
    asm volatile (" mrs %0, IPSR " : "=r" (exnum));     // get exception number      
    auto head = get_head(exnum);
    if(!head || !head->link_)
        for(;;);    // no ISR, hang here
    // scan through the ring and call isr() of all entries in turn
    for(auto h = head->link_; h && h != head; h = h->link_)
        h->isr();
}

template<size_t N, size_t M> constexpr std::array<arm::Interrupt::VectorTableEntry*, N>
make_vector_table(std::array<arm::Interrupt::VectorTableEntry*, M> initial_entries) {
    std::array<arm::Interrupt::VectorTableEntry*, N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = (i < M) ? initial_entries[i] : &arm::Interrupt::defaultISR;
    return result;
}

constexpr std::array<arm::Interrupt::VectorTableEntry*, 16> specific_handlers = {
    reinterpret_cast<arm::Interrupt::VectorTableEntry*>(&__stack),  // The initial stack pointer
    &ResetISR,                      //  1: Reset
    &arm::Interrupt::defaultISR,    //  2: NMI
    &arm::Interrupt::defaultISR,    //  3: HardFault
    &arm::Interrupt::defaultISR,    //  4: MemManageFault
    &arm::Interrupt::defaultISR,    //  5: BusFault
    &arm::Interrupt::defaultISR,    //  6: UsageFault
    &arm::Interrupt::defaultISR,    //  7: SecureFault
    nullptr,                        //  8: reserved
    nullptr,                        //  9: reserved
    nullptr,                        // 10: reserved
    &arm::Interrupt::defaultISR,    // 11: SVCall
    &arm::Interrupt::defaultISR,    // 12: DebugMonitor
    nullptr,                        // 13: reserved
    &arm::Interrupt::defaultISR,    // 14: PendSV
    &arm::Interrupt::defaultISR     // 15: SysTick
};

__attribute__((used, section(".isr_vector")))
std::array<arm::Interrupt::VectorTableEntry *, i_NVIC.interrupts + 16> const vector_table
    = make_vector_table<interrupt_table.size(), specific_handlers.size()>(specific_handlers);
