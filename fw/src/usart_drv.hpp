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
    namespace USART {
        struct Intgr;
    }

/** USART Driver.
 */
class Usart {
    Usart(Usart &&) = delete;
public:

    size_t send(void const *buf, size_t size);

    void isr();

    Usart(USART::Intgr const &in);
    ~Usart() =default;

private:
    USART::Intgr const &in_;
};

} // namespace

//!@}
