/** @file
 * FTM driver.
 * 
 * @addtogroup LPC865_ftm
 * @ingroup LPC865
 * @{
 */

#pragma once

#include "FTM.hpp"
#include <span>
#include <cstddef>
#include <cstdint>

namespace lpc865 {

struct Event;

/** FTM driver.
 * 
 * This is a generic interface for the FTM timer. The following
 * features can be supported:
 * - Multiple channels
 * - Quadrature encoder
 */
class Ftm {
    Ftm(Ftm &&) = delete;
public:
    ~Ftm() =default;

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
        uint32_t ps:3;      //!< Prescaler factor (power of 2)
        uint32_t clks:2;    //!< Clock source: 0 = none, 1 = input, 2 = qdec, 3 = external
        uint32_t updn:1;    //!< Counter counts up and down
        uint32_t _0:10;     //!< reserved
        uint32_t mod:16;    //!< Counter modulus (value where it resets)
        uint32_t ch0inv:1;  //!< Channel 0 inverted output
        uint32_t ch1inv:1;  //!< Channel 1 inverted output
        uint32_t ch2inv:1;  //!< Channel 2 inverted output
        uint32_t ch3inv:1;  //!< Channel 3 inverted output
        uint32_t ch4inv:1;  //!< Channel 4 inverted output
        uint32_t ch5inv:1;  //!< Channel 5 inverted output
        uint32_t ch6inv:1;  //!< Channel 6 inverted output
        uint32_t ch7inv:1;  //!< Channel 7 inverted output
        uint32_t ch0:3;     //!< Channel 0 mode (see enum Mode)
        uint32_t ch1:3;     //!< Channel 1 mode (see enum Mode)
        uint32_t ch2:3;     //!< Channel 2 mode (see enum Mode)
        uint32_t ch3:3;     //!< Channel 3 mode (see enum Mode)
        uint32_t ch4:3;     //!< Channel 4 mode (see enum Mode)
        uint32_t ch5:3;     //!< Channel 5 mode (see enum Mode)
        uint32_t ch6:3;     //!< Channel 6 mode (see enum Mode)
        uint32_t ch7:3;     //!< Channel 7 mode (see enum Mode)
    };

    Ftm(FTM::Integration const &in, Parameters const &par);

private:
    FTM::Integration const &in_;
};

} // namespace

//!@}
