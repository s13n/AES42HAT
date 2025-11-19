/** @file
 * Generic handler interface
 * 
 * @addtogroup Channel
 * @ingroup AES42HAT
 * @{
 */

#pragma once

#include <cstddef>
#include <cstdint>

class Handler {
    Handler(Handler &&) =delete;
protected:
    ~Handler() =default;
    Handler() =default;
public:
    virtual void act() =0;

    bool post();

    [[noreturn]] static void run();
    static size_t poll();
    static size_t poll_one();

private:
    static Handler *unque();

    Handler *next_;
};
