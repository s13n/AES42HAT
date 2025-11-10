/** @file
 * main function for the AES42HAT
 */

#include "spi_drv.hpp"
#include "WKT.hpp"
#include "WWDT.hpp"
#include "LPC865.hpp"
#include <stdint.h>

using namespace lpc865;

static Spi spi0{ i_SPI0, nullptr };

int main() {
    auto &gpio = *i_GPIO.registers;     // GPIO register set
    auto &wkt = *i_WKT.registers;       // WKT register set

    gpio.DIRSET[1].set(1 << 7);
    while (1) {
        gpio.NOT[1].set(1 << 7); // LED toggle

        wkt.CTRL.set(6);
        wkt.COUNT.set(5000);
        while (wkt.CTRL.get().ALARMFLAG == NO_TIME_OUT);
    }
    return 0;
}
