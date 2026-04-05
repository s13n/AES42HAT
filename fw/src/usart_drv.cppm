/** @file
 * driver for the LPC8 USART block.
 *
 * @addtogroup LPC865_usart
 * @ingroup LPC865
 * @{
 */

module;
#include <cstddef>
#include <cstdint>
export module usart_drv;
import USART;

export namespace lpc865 {

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
