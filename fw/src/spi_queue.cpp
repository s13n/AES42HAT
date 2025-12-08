/** @file
 * work queue for the SPI.
 * @addtogroup LPC865_spi
 * @ingroup LPC865
 * @{
 */
#include "spi_queue.hpp"
#include <bit>


void lpc865::SpiQueue::enqueue(Entry &e) {
    bool empty = queue_.empty();
    queue_.push_back(e);
    if (empty)
        handle_next();
}

void lpc865::SpiQueue::act() {
    auto &e = queue_.front();
    if (e.hdl)
        e.hdl->act();
    queue_.pop_front();
    if (!queue_.empty())
        handle_next();
}

lpc865::SpiQueue::SpiQueue(Spi &spi)
    : spi_{spi}
{
}

void lpc865::SpiQueue::handle_next() {
    auto &e = queue_.front();
    spi_.target(e.par, this);
    spi_.transfer(e.buf, e.size, e.speed);
}

/** @}*/
