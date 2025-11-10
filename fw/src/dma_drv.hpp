/** @file
 * LPC865 dma driver.
 * @addtogroup LPC865_dma
 * @ingroup LPC865
 * @{
 */
#pragma once

#include "DMA.hpp"
#include "utility.hpp"
#include <cassert>
#include <span>

namespace lpc865 {

//! Driver for the LPC8 DMA controller.
class DmaBase {
    DmaBase(DmaBase &&) = delete;

public:
    using Buffer = std::span<std::byte>;
    using BufferList = std::span<Buffer>;

    struct Channel {
        Channel(DmaBase &dma);

        bool init(DMA::CFG cfg, DMA::XFERCFG xfer, intptr_t addr);
        bool start(BufferList);
        void stop();
        bool busy();
        bool completed();

        struct Descriptor {
            DMA::XFERCFG xfer;  //!< Transfer configuration.
            uint32_t src;       //!< Source end address. This points to the address of the last entry of the source address range if the address is incremented. The address to be used in the transfer is calculated from the end address, data width, and transfer size.
            uint32_t dst;       //!< Destination end address. This points to the address of the last entry of the destination address range if the address is incremented. The address to be used in the transfer is calculated from the end address, data width, and transfer size.
            uint32_t link;      //!< Link to next descriptor. If used, this address must be aligned to a multiple of 16 bytes (i.e., the size of a descriptor).
        };

        DmaBase &dma_;      //!< Link to parent object
    };

    ~DmaBase();
    DmaBase(DMA::Integration const &in, void *mem, size_t size);

    void isr();

    //! set up a transfer on the given channel
    /** The transfer details are given in the descriptor chain pointed to by parameter desc.
     * The transfer starts immediately, controlled by the selected handshaking method.
     * Note that the DMA request multiplexing must have been set up before.
     * @param chan channel number
     * @param cfg this gets written to the channel configuration register
     * @param xfer this gets written to the channel transfer configuration register
     * @param addr The peripheral's data register address.
     * @return Channel ready to use. nullptr if channel is unavailable or configuration is wrong.
     */
    Channel *setup(unsigned chan, DMA::CFG cfg, DMA::XFERCFG xfer, intptr_t addr);
    
private:
    friend struct Channel;

    DMA::Integration const &in_;            //!< Integration parameters
    std::span<Channel::Descriptor> desc_;   //!< Channel descriptor memory
};

template<size_t Ch> class Dma : public DmaBase {
    std::array<Channel, Ch> channels_;

public:
    Dma(DMA::Integration const &in)
        : DmaBase(in)
        , channels_{make_array<Ch, Channel>(*this)}
    {
        assert(Ch == in.max_channel+1);
    }
};

} //!@} namespace
