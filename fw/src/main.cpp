
#include <stdint.h>
#define GPIO0_BASE 0xA0000000
#define GPIO_DIR (*(volatile uint32_t*)(GPIO0_BASE + 0x2004))
#define GPIO_SET (*(volatile uint32_t*)(GPIO0_BASE + 0x2204))
#define GPIO_CLR (*(volatile uint32_t*)(GPIO0_BASE + 0x2284))
#define SYSAHBCLKCTRL0 (*(volatile uint32_t*)(0x40048080))

extern "C" void delay() {
    for (volatile int i = 0; i < 300000; ++i);
}

int main() {
    SYSAHBCLKCTRL0 = 0x31FFFEF7;
    GPIO_DIR |= (1 << 7); // Set PIO1_7 as output
    while (1) {
        GPIO_SET = (1 << 7); // LED on
        delay();
        GPIO_CLR = (1 << 7); // LED off
        delay();
    }
    return 0;
}
