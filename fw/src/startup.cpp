
#include <stdint.h>

extern int main();
extern "C" void Reset_Handler() {
    main();
}

__attribute__((section(".isr_vector")))
const void* vector_table[] = {
    (void*)0x10002000, // Initial stack pointer
    (void*)Reset_Handler,
};
