/** @file
 * driver for the LPC8 WKT.
 *
 * @addtogroup LPC865_wkt
 * @ingroup LPC865
 * @{
 */

module;
#include <cstddef>
#include <cstdint>
export module wkt_drv;
import nvic_drv;
import handler;
import WKT;

export namespace lpc865 {

/** WKT Driver.
 */
class Wkt : public arm::Interrupt {
public:
    struct Parameters {
        uint8_t clksel:1;   // Select internal clock source. 0 = FRO; 1 = ULPCLK
        uint8_t extclk:1;   // Select clock source. 0 = internal; 1 = external (WKTCLKIN)
    };

    void isr() override;

    void start(uint32_t count, Handler &hdl);

    Wkt(WKT::Intgr const &in, Parameters const &par);
    ~Wkt() =default;

private:
    WKT::Intgr const &in_;  //!< Integration values
    Handler *hdl_;
};

} // namespace

//!@}
