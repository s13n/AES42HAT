/** @file
 * SPI controller driver.
 * 
 * @addtogroup LPC865_spi
 * @ingroup LPC865
 * @{
 */

#pragma once

#include "handler.hpp"
#include "nvic_drv.hpp"
#include <cstddef>
#include <cstdint>

namespace lpc865 {

class Dma;

inline namespace SPI {
    struct Integration;
}

/** SPI driver.
 * This is a generic interface for the controller of an SPI port. The following
 * features can be supported:
 * - Multiple target selects.
 */
class Spi : public arm::Interrupt, public Handler {
public:
    /** Bus type. Starting with SPI in the 1980s, additional variants for higher
     * bandwidth have been devised over and above what basic SPI offers. SPI
     * controllers typically support a subset of them. This enum encodes the
     * variant used. The basic SPI port comes in different widths, i.e. data
     * wires. Additional variants differ in clocking and strobing arrangements,
     * they are:
     * - HyperBus was defined by Cypress, now Infineon. See their HyperBus
     *   Specification.
     * - Xccela was defined by Micron.
     * - xSPI was standardised by JEDEC as standard 251C
     *
     * Note that HyperBus, Xccela and xSPI are similar 8-bit wide buses based on
     * SPI, with the extra provision of differential clocking and/or a separate
     * data strobe signal to allow for greater bandwidth. Specifically, HyperBus
     * and xSPI offer the option of using a differential clock, while Xccela
     * doesn't. Accesses in HyperBus must be 16-bit aligned, while Xccela offers
     * 8-bit alignment.
     * 
     * Single-bit SPI features 4 different modes, which differ in SCK sampling
     * edge, and SCK idle polarity (multi-bit SPI always assumes mode 0):
     * - Mode 0: CPOL=0, CPHA=0; Clock idles low, rising edge sampling
     * - Mode 1: CPOL=0, CPHA=1; Clock idles low, falling edge sampling
     * - Mode 2: CPOL=1, CPHA=0; Clock idles high, falling edge sampling
     * - Mode 3: CPOL=1, CPHA=1; Clock idles high, rising edge sampling
     */
    enum BusType {
        spiMode0 = 0,   //!< single-bit classic SPI Mode 0, full duplex capable
        spiMode1,       //!< single-bit classic SPI Mode 1, full duplex capable
        spiMode2,       //!< single-bit classic SPI Mode 2, full duplex capable
        spiMode3,       //!< single-bit classic SPI Mode 3, full duplex capable
        spi2,           //!< dual SPI, two bits parallel
        spi4,           //!< quad SPI, four bits parallel
        spi8,           //!< octal SPI, eight bit parallel
        spi16,          //!< hex SPI, sixteen bit parallel
        hyperbus,       //!< HyperBus, 8-bit parallel
        xccela,         //!< Xccela, 8-bit parallel
        xSPI,           //!< JEDEC 251C xSPI, 8-bit parallel
    };

    /** Data pin usage in command transaction.
     * The values encode pin usage for instruction, address and data, where the digit denotes the
     * number of data pins used for the respective part in the command frame. The letter encodes if
     * single (S) or double (D) data rate is used at that point.
     * 
     * Example: 1S-4D-4D means that a single data pin is used to transfer the instruction byte with
     * the rising clock edge for each bit, and the address and data bytes are then transferred using
     * 4 data pins, with both clock edges used.
     * 
     * Note that there is no information about the maximum clock frequency, or any extra (dummy) clock
     * cycles needed. This is either implicit in the command used, or requires further information to
     * be obtained.
     * 
     * The list isn't meant to be exhaustive. We only list the modes that are most useful, and most
     * likely supported by both memory device and controller.
     * 
     * Be careful when altering the list, the ordering matters!
     */ 
    enum PinUsage {
        puUnspecified = 0,
        pu1S1S1S,   //!< 1S-1S-1S
        pu1S2S2S,   //!< 1S-2S-2S
        pu2S2S2S,   //!< 2S-2S-2S
        pu1S4S4S,   //!< 1S-4S-4S
        pu4S4S4S,   //!< 4S-4S-4S
        pu1S4D4D,   //!< 1S-4D-4D
        pu4S4D4D,   //!< 4S-4D-4D
        pu4D4D4D,   //!< 4D-4D-4D
        pu1S8S8S,   //!< 1S-8S-8S
        pu8S8S8S,   //!< 8S-8S-8S
        pu1S8D8D,   //!< 1S-8D-8D
        pu8D8D8D,   //!< 8D-8D-8D
    };

    /** Values for the 4-bit maxHz field.
     * The entries are derived from the JEDEC JESD216 standard, chapter 6.4.23.
     */
    enum MaxSpeed {
        mHzUndef = 0,
        mHz33,
        mHz50,
        mHz66,
        mHz80,
        mHz100,
        mHz133,
        mHz166,
        mHz200,
        mHz250,
        mHz266,
        mHz333,
        mHz400,
    };

    /** Parameters describing a command.
     * 
     * Note regarding the read and write flags: Both can be set at the same time
     * to make use of the full duplex operation of ordinary SPI. Some SPI
     * controllers don't support this, however. If both flags are cleared, there
     * is no data phase.
     */
    struct CommandDescriptor {
        uint32_t pu:4;      //!< Pin usage code (see enum PinUsage)
        uint32_t maxHz:4;   //!< Maximum frequency code (see enum MaxSpeed)
        uint32_t read:1;    //!< Data phase uses read direction
        uint32_t write:1;   //!< Data phase uses write direction
        uint32_t needen:1;  //!< Needs enable command to be sent first
        uint32_t busy:1;    //!< Device will be busy after this command
        uint32_t ax:1;      //!< Supports AX mode (continuous read / XIP / ...)
        uint32_t addb:3;    //!< Number of address bytes
        uint32_t dummy:5;   //!< Number of dummy clocks needed before valid data
        uint32_t mclks:3;   //!< Number of clocks needed for "mode" bits.
        uint32_t ins:8;     //!< Instruction code
    };

    /** Parameters for a transfer.
     *
     * This struct collects parameters for supporting SPI controllers that may
     * support multi-bit modes, especially QSPI or HyperBus.
     *
     * Note that depending on the bus type, the extension byte either follows
     * the instruction or the address.
     *
     * Up to 8 target select pins can be activated, if the hardware supports it.
     * In the simplest case, only 1 target select pin is supported, in which
     * case it is controlled by the LSB of field `sel`. Some SPI controllers
     * support multiple target select pins, and sometimes more than one can be
     * active simultaneously (which allows external decoding).
     * 
     * Polling can be used to determine if the memory chip has completed its
     * internal operation. This is useful for flash chips which are busy for
     * a while after issuing a command, for example when erasing a block or
     * programming a page. Two different methods can be supported:
     * 1. Polling the status register with command 0x05, and observing bit 0
     * 2. Polling the flag status register with command 0x70, and observing bit 7
     * 
     * Some memory chips support both, but you need to select one. Note that
     * the two methods have the opposite polarity of the observed bit.
     */
    struct Parameters {
        CommandDescriptor cmd;
        uint32_t type:4;    //!< Bus type to use (see enum BusType)
        uint32_t twin:1;    //!< Paired chips are used. Note that the bus type given above is for one chip only.
        uint32_t noins:1;   //!< Instruction not issued (0-4-4 mode)
        uint32_t poll:2;    //!< Poll for completion after write command. 0: don't; 1: Use cmd 0x05 bit 0; 2: Use cmd 0x70 bit 7; 3: res
        uint32_t noinsen:1; //!< 0-4-4 mode must be enabled by setting bit 3 in the volatile configuration register (e.g. MT25QL512ABB)
        uint32_t res:7;
        uint32_t ext:8;     //!< Extension byte
        uint32_t sel:8;     //!< Up to 8 target selects
    };

    /** Select a target and define its transfer properties.
     *
     * Subsequent transfers are addressed to the selected target. This may only
     * be called while there is no transfer in progress.
     * @param par Transfer parameters to set up interface
     * @param hdl The event handler to post on transfer completion. May be nullptr,
     *   for no notification.
     * @return true if successful, false if bus busy or setting unsupported.
     *
     * The SPI controller is configured to select the given target and to
     * perform transfers using the given parameters. The bus itself isn't yet
     * used, and no transfer is started, so the bus is remaining idle after this
     * call. After the call returns true, a transfer can be invoked immediately.
     * 
     * Note that multiple transfers can be done without calling `target()` in
     * between. Those transfers will all use the same parameters. If `sioo` is
     * set, the instruction will only be sent on the first transfer after
     * `target()` returns. Subsequent transfers omit the instruction. This is a
     * way to increase throughput that is supported by some devices and drivers.
     * For getting the device out of this mode, a device specific command must
     * be issued by the user, i.e. the driver won't do it automatically.
     * 
     * If the given parameters can't be supported, false is returned.
     * 
     * Note that setting both the `read` and the `write` bit in `par` calls for
     * full duplex transfers, which are only possible in single-bit SPI mode.
     * The `transfer()` function must be used for full-duplex transfers.
     * 
     * Note that a transfer is complete when the device can perform a new
     * operation. This means that when writing data or issuing programming
     * commands to a flash device, you should only post the event `ev` when the
     * operation has completed. This is typically done by polling a status
     * register of the memory device, which in some cases the controller can do
     * autonomously, and issue an interrupt on completion. Without hardware
     * support, the software must do the polling, while servicing the event
     * queue in order to not block the application.
     */
    bool target(Parameters const &par, Handler *hdl);

    /** Perform an asynchronous bidirectional transfer.
     *
     * This function is performing basic SPI type transfers, with no address
     * support. It is meant to be used with non-memory type target devices, or
     * with devices that require full-duplex communication.
     * 
     * @param buf source and/or destination buffer for the data to transfer
     * @param size Number of bytes to transfer
     * @param speed Desired bitclock frequency in Hz
     * @return number of bytes transferred if successful, negative error code otherwise
     *
     * If the given speed is 0, the maxHz parameter given to the previous
     * target() call will be used to determine the bit clock frequency. A
     * value > 0 will override the maxHz parameter, and allows finer-grained
     * control over the frequency, for applications that require a particular
     * timing. The actual frequency used will be the next smaller frequency
     * supported by the SPI controller hardware.
     * 
     * Note that the function may not wait for the transfer to complete, if an
     * Event was set in the previous target() call. In this case it returns
     * -EINPROGRESS to indicate that completion will be signaled through the
     * Event. If no Event was set, the call will return after completion.
     * The caller may not touch the buffer passed, until the transfer has
     * completed, and must ensure that the buffer remains allocated until then.
     * The transfer proceeds in the background, by means of DMA and/or
     * interrupts. When completed, the notification occurs by posting an event
     * handler.
     * 
     * Bidirectional data transfer is supported by setting both `read` and
     * `write` bits in the parameters passed to `target()`. The data that was
     * read replaces the data written in the buffer. Note that this is only
     * supported for single-bit SPI transfers. If the last `target()` call
     * didn't set parameters that are compatible with basic SPI transfers, the
     * call fails. Most importantly, `type == spi` must be true.
     */
    ptrdiff_t transfer(void *buf, size_t size, uint32_t speed=0);

    enum Status {
        uninitialized,  //!< Controller is not initialized or disabled
        error,          //!< Controller is in an error state
        idle,           //!< Controller is ready for new commands
        busy,           //!< Controller is operating on the previous command
    };

    /** Return the current bus status.
     */
    Status status() const;

    Spi(SPI::Integration const &in, Dma *dma);
    ~Spi() =default;

    void act() override;
    void isr() override;

private:
    SPI::Integration const &in_;
    Dma *dma_;
    Handler *hdl_;
};

} // namespace

//!@}
