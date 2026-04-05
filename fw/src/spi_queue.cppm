/** @file
 * work queue for the SPI.
 *
 * @addtogroup LPC865_spi
 * @ingroup LPC865
 * @{
 */

module;
#include <cstddef>
#include <cstdint>
export module spi_queue;
import spi_drv;
import queuering;
import handler;

export namespace lpc865 {

/** WKT Driver.
 */
class SpiQueue : public Handler {
public:
    struct Entry {
        friend QueueRing<Entry>::pointer &next(QueueRing<Entry>::const_reference e) {
            return const_cast<QueueRing<Entry>::pointer&>(e.next);
        }

        Spi::Parameters par = {};
        void *buf = nullptr;
        size_t size = 0;
        uint32_t speed = 0;
        Handler *hdl = nullptr;     // completion handler
        Entry *next = nullptr;      // for forming linked list of entries
    };

    void enqueue(Entry &e);

    void act() override;

    explicit SpiQueue(Spi &spi)
        : spi_{spi}
    {
    }

private:
    void handle(Entry &e);

    Spi &spi_;
    QueueRing<Entry> queue_;
};

} // namespace

//!@}
