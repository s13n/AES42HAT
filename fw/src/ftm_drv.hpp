/** @file
 * FTM driver.
 * 
 * @addtogroup LPC865_ftm
 * @ingroup LPC865
 * @{
 */

#pragma once

#include "nvic_drv.hpp"
#include <span>
#include <cstddef>
#include <cstdint>

struct Handler;

namespace lpc865 {

inline namespace FTM {
    struct Integration;
}

/** FTM driver.
 * 
 * This is a generic interface for the FTM timer. The following
 * features can be supported:
 * - Multiple channels
 * - Quadrature encoder
 */
class Ftm : public arm::Interrupt {
    Ftm(Ftm &&) = delete;
public:
    ~Ftm() =default;

    /** Get the current count.
     * @return current count value
     */
    uint16_t getCount();

    /** Set the match value that defines the PWM duty cycle.
     * @param ch Channel number
     * @param value Match value
     * 
     * The exact behavior depends on the channel mode. For example in edge
     * PWM mode, the match value defines the point in time when the output
     * gets reset. The counter overflow defines the point whe it gets set.
     */
    void setMatch(unsigned ch, uint16_t value);

    /** Get the capture value of the given channel.
     * @param ch Channel number
     * @return Last capture value
     */
    uint16_t getCapture(unsigned ch);

    /** Set interrupt handlers.
     * @param overflow Handler for overflow interrupt
     * @param reload Handler for reload interrupt
     * 
     * When a non-null pointer is passed for any of the handlers, the corresponding
     * interrupt is enabled, and the handler will be called in interrupt context.
     * When a null pointer is passed, the corresponding interrupt is disabled.
     */
    void setHandlers(Handler *overflow, Handler *reload);

    void setModulusDelta(int16_t delta);

    /** Operating mode of a capture/compare channel.
     * 
     * Note that combine mode can only selected on an even numbered channel. It uses the
     * next channel in combination, for separate control of rising and falling edge position,
     * generating a complementary output on the two output pins.
     */
    enum Mode {
        off,            //!< Channel unused
        capturePos,     //!< Channel captures on rising edge
        captureNeg,     //!< Channel captures on negative edge
        captureBoth,    //!< Channel captures on either edge
        pwmCombine,     //!< PWM output combining two channels (ch n sets, ch n+1 clears)
        compareToggle,  //!< Channel compare toggles output
        pwmPos,         //!< positive PWM output (rising on overflow, falling on compare)
        pwmNeg,         //!< negative PWM output (falling on overflow, rising on compare)
    };

    /** Operating parameters for the FTM. */
    struct Parameters {
        uint16_t ps:3;      //!< Prescaler factor (power of 2)
        uint16_t clks:2;    //!< Clock source: 0 = none, 1 = input, 2 = fixed, 3 = external
        uint16_t updn:1;    //!< 1: Counter counts up and down (PWM is center-aligned)
        uint16_t ovint:1;   //!< Counter overflow interrupt enable
        uint16_t rlint:1;   //!< Counter reload interrupt enable
        uint16_t init;      //!< Counter initial value
        uint16_t mod;       //!< Counter modulus (value where it resets)
        uint16_t hcyc;      //!< Half cycle reload value (= reload opportunity)
        struct Channel {
            uint8_t mode:3; //!< Channel mode (see enum Mode)
            uint8_t inv:1;  //!< Inverted output
            uint8_t trig:1; //!< Channel outputs trigger pulse
            uint8_t intr:1; //!< Channel interrupt enable
        } ch[8];
    };

    Ftm(FTM::Integration const &in, Parameters const &par);

private:
    void isr() override;

    uint16_t mod_;
    FTM::Integration const &in_;
    Handler *overflow_;
    Handler *reload_;
};

} // namespace

//!@}
