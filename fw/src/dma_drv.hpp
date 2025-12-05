/** @file
 * LPC865 dma driver.
 * @addtogroup LPC865_dma
 * @ingroup LPC865
 * @{
 */
#pragma once

#include "nvic_drv.hpp"
#include "utility.hpp"

struct Handler;

namespace lpc865 {

inline namespace SmartDMA {
    struct Integration;
}

//! Driver for the LPC8 DMA controller.
class Dma : public arm::Interrupt {
public:
    /** DMA channel settings pertaining to the peripheral served. */
    struct Per {
        uint32_t chan:6;        //!< Channel number (up to 63 to account for future extension)
        uint32_t width:2;       //!< Data transfer width. 0: 8-bit, 1: 16-bit, 2: 32-bit, 3: reserved
        uint32_t dest:1;        //!< 0: peripheral is source, 1: peripheral is destination
        uint32_t hwtrig:1;      //!< Hardware triggering enabled
        uint32_t trigpol:1;     //!< Hardware trigger is active high 
        uint32_t trigtype:1;    //!< Hardware trigger is level triggered
        uint32_t trigburst:1;   //!< Hardware trigger causes a burst transfer
    };

    /** DMA channel settings pertaining to the memory buffer served. */
    struct Mem {
        uint32_t chan:6;        //!< Channel number (up to 63 to account for future extension)
        uint32_t inc:2;         //!< Memory address increment. 0: none, 1: 1*width, 2: 2*width, 3: 4*width
        uint32_t burstpower:4;  //!< Burst power. Burst size is 2 raised to this number
        uint32_t burstwrap:1;   //!< Memory address is wrapped
        uint32_t prio:3;        //!< Channel priority
        uint32_t setintA:1;     //!< Set interrupt flag A upon exhaustion of descriptor
        uint32_t setintB:1;     //!< Set interrupt flag B upon exhaustion of descriptor
    };

    struct Descriptor {
        uint32_t xfer;          //!< Transfer configuration.
        uint32_t src;           //!< Source end address. This points to the address of the last entry of the source address range if the address is incremented. The address to be used in the transfer is calculated from the end address, data width, and transfer size.
        uint32_t dst;           //!< Destination end address. This points to the address of the last entry of the destination address range if the address is incremented. The address to be used in the transfer is calculated from the end address, data width, and transfer size.
        uint32_t link;          //!< Link to next descriptor. If used, this address must be aligned to a multiple of 16 bytes (i.e., the size of a descriptor).
    };

    struct Parameters {
        Descriptor *descs;      //!< Pointer to array of descriptors
    };

    /** Set up a peripheral transfer on the given channel.
     * The transfer details are given in the descriptor chain pointed to by parameter desc.
     * The transfer starts immediately, controlled by the selected handshaking method.
     * Note that the DMA request multiplexing must have been set up before.
     * @param per peripheral transfer properties
     * @param addr The peripheral's data register address.
     * @param hdl Optional handler to call on transfer termination
     * @return true if channel is ready to use. false if channel is unavailable or configuration is wrong.
     */
    bool setup(Per per, uintptr_t addr, Handler *hdl);

    /** Start a peripheral transfer on the given channel.
     * The transfer details are given in the descriptor chain pointed to by parameter desc.
     * The transfer starts immediately, controlled by the selected handshaking method.
     * Note that the DMA request multiplexing must have been set up before.
     * @param per peripheral transfer properties
     * @param addr The peripheral's data register address.
     * @return true if channel is ready to use. false if channel is unavailable or configuration is wrong.
     */
    bool start(Mem mem, void *buf, size_t size);

    ~Dma();
    Dma(SmartDMA::Integration const &in, Parameters const &par);

    void isr() override;
    
private:
    SmartDMA::Integration const &in_;   //!< Integration parameters
    Parameters const &par_;
};

} //!@} namespace
