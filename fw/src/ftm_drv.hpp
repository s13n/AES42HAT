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
 * This is a generic interface for the FTM timer. The following
 * features can be supported:
 * - Multiple channels
 * - Quadrature encoder
 */
class Ftm {
    Ftm(Ftm &&) = delete;
public:
    /** Operating parameters for the FTM. */
    struct Parameters {
        uint32_t ps:3;      //!< Prescaler factor (power of 2)
        uint32_t clks:2;    //!< Clock source: 0 = none, 1 = input, 2 = qdec, 3 = external
    };

    Ftm(FTM::Integration const &in, Parameters const &par);
    ~Ftm() =default;

private:
    FTM::Integration const &in_;
};

} // namespace

//!@}
