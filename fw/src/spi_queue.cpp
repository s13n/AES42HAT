/** @file
 * work queue for the SPI.
 * @addtogroup LPC865_spi
 * @ingroup LPC865
 * @{
 */
#include "spi_queue.hpp"


void lpc865::SpiQueue::enqueue(Entry &e) {
    bool empty = queue_.empty();
    queue_.push_back(e);
    if (empty)
        handle(e);
}

void lpc865::SpiQueue::act() {
    auto &e = queue_.front();
    if (e.hdl)
        e.hdl->act();
    queue_.pop_front();
    if (!queue_.empty())
        handle(queue_.front());
}

void lpc865::SpiQueue::handle(Entry &e) {
    spi_.target(e.par, this);
    spi_.transfer(e.buf, e.size, e.speed);
}

/** @}*/
