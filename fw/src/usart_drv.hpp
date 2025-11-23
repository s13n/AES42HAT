/** @file
 * driver for the LPC8 USART block.
 * 
 * @addtogroup LPC865_usart
 * @ingroup LPC865
 * @{
 */

#pragma once

#include "USART.hpp"
#include <cstddef>
#include <cstdint>

namespace lpc865 {

struct Event;

/** USART Driver.
 */
class Usart {
    Usart(Usart &&) = delete;
public:

    size_t send(void const *buf, size_t size);

    void isr();

    Usart(USART::Integration const &in);
    ~Usart() =default;

private:
    USART::Integration const &in_;
};

} // namespace

//!@}
