/** @file
 * driver for the LPC8 USART block.
 * @addtogroup LPC865_usart
 * @ingroup LPC865
 * @{
 */
#include "usart_drv.hpp"


void lpc865::Usart::isr() {
    
}

lpc865::Usart::Usart(USART::Integration const &in)
    : in_{in}
{
    auto &hw = *in_.registers;
}

/** @}*/
