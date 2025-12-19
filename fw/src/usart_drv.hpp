/** @file
 * driver for the LPC8 USART block.
 * 
 * @addtogroup LPC865_usart
 * @ingroup LPC865
 * @{
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace lpc865 {
    namespace integration {
        struct USART;
    }

/** USART Driver.
 */
class Usart {
    Usart(Usart &&) = delete;
public:

    size_t send(void const *buf, size_t size);

    void isr();

    Usart(integration::USART const &in);
    ~Usart() =default;

private:
    integration::USART const &in_;
};

} // namespace

//!@}
