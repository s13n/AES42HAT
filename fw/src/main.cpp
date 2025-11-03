/** @file
 * main function for the AES42HAT
 */

#include "WKT.hpp"
#include "WWDT.hpp"
#include "LPC865.hpp"
#include <stdint.h>

extern "C" void delay() {
    for (volatile int i = 0; i < 300000;) {
        int x = i;
        i = x + 1;
    }
}

using namespace lpc865;

int main() {
    auto &gpio = *i_GPIO.registers;         // GPIO register set

    gpio.DIRSET[1].set(1 << 7);
    while (1) {
        gpio.NOT[1].set(1 << 7); // LED toggle
        delay();
    }
    return 0;
}
