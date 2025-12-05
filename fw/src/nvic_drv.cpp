/** @file
 * ARM NVIC driver
 * 
 * @addtogroup ARM_NVIC
 * @ingroup ARM
 * @{
 */

#include "nvic_drv.hpp"
#include "LPC865.hpp"
#include <array>
extern "C" {
#   include "newlib_def.h"
}

extern void setActivityLED(bool act);

using namespace lpc865;

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

/** Table of objects that handle the respective exception.
 * 
 * Each table entry serves as the head of a ring of linked objects. This allows
 * several objects to share an interrupt. When an interrupt occurs, the `isr()`
 * function members of all objects are called in sequence. Each object needs
 * to check if it has any work to do.
 */
static std::array<arm::Interrupt *, i_NVIC.interrupts + 16> interrupt_table = {};

/** get a pointer to the ring head for the given exception number
 * @param n Exception number
 * @return Pointer to ring head, or nullptr if exception number was invalid
 */
arm::Interrupt *arm::Interrupt::get_head(Exception n) {
    // We use some ugly casts to simulate a list head that looks like an
    // object of type InterruptHandler, even though it is only a pointer.
    // This simplifies the list insertion, deletion and traversal operations.
    if(n >= interrupt_table.size() || n == 0)
        return nullptr;
    auto offset = (char*)&(interrupt_table[n]->link_) - (char*)(interrupt_table[n]);
    auto object = (char*)(&interrupt_table[n]) - offset;
    return reinterpret_cast<arm::Interrupt *>(object);
}

void arm::Interrupt::insert(Exception n) {
    // We don't disable interrupts here, so interrupts may occur at any time.
    // This works because interrupt service routines are not allowed to add or remove entries
    // from the ring; the only thing that can happen on an interrupt is ring traversal.
    // The code below changes the ring by merely one atomic write to h->next_, so any
    // interrupt that happens to occur while the code below is running, either finds this
    // write has already happened, in which case the new interrupt service routine gets called,
    // or it finds it hasn't happened yet, in which case the new interrupt service routine
    // doesn't get called yet.
    // Keep this in mind and be very careful when changing the code below.
    if(auto h = get_head(n)) {
        link_ = h->link_ ? h->link_ : h;
        h->link_ = this;
        if(link_ == h)
            enable(n);  // this is the first handler --> enable the interrupt
    }
}

void arm::Interrupt::enable(Exception n) {
    if(n >= interruptOffset) {
        unsigned index = (n-interruptOffset) >> 5;
        uint32_t mask = 1 << ((n-interruptOffset) & 0x1F);
        i_NVIC.registers->ISER[index] = mask;
    }
}

void arm::Interrupt::disable(Exception n) {
    if(n >= interruptOffset) {
        unsigned index = (n-interruptOffset) >> 5;
        uint32_t mask = 1 << ((n-interruptOffset) & 0x1F);
        i_NVIC.registers->ICER[index] = mask;
    }
}

/** Default interrupt service routine.
 * 
 * This routine dispatches the interrupt call to the object(s) registered in the
 * `interrupt_table`. It is the routine that is used in the vector table, unless
 * a different routine is specified in the `specific_handlers` array.
 */
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
    setActivityLED(true);
    if(!head || !head->link_)
        for(;;);    // no ISR, hang here
    // scan through the ring and call isr() of all entries in turn
    for(auto h = head->link_; h && h != head; h = h->link_)
        h->isr();
}

/** Vector table initialization function.
 * @tparam N Number of entries in the vector table
 * @tparam M Number of special entries at the beginning 
 * @param initial_entries The array with the special entries at the start of the vector table.
 * @return The initialization data for the actual vector table.
 * 
 * This is a compile-time function template that is used to initialize the vector table
 * in a generic way, taking into account a table of specific entries at the beginning,
 * and a default handler for all the other entries.
 */
template<size_t N, size_t M> constexpr std::array<arm::Interrupt::VectorTableEntry*, N>
make_vector_table(std::array<arm::Interrupt::VectorTableEntry*, M> initial_entries) {
    std::array<arm::Interrupt::VectorTableEntry*, N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = (i < M) ? initial_entries[i] : &arm::Interrupt::defaultISR;
    return result;
}

/** The interrupt vector table.
 * 
 * The table needs to be placed at the right address in memory, which is achieved by
 * putting it into a special linker section. See the linker script for where it is
 * placed.
 */
__attribute__((used, section(".isr_vector")))
std::array<arm::Interrupt::VectorTableEntry *, i_NVIC.interrupts + 16> const vector_table
    = make_vector_table<interrupt_table.size(), specific_handlers.size()>(specific_handlers);
