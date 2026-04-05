/** @file
 * Generic handler interface
 *
 * @addtogroup Channel
 * @ingroup AES42HAT
 * @{
 */

module;
#include <cstddef>
#include <cstdint>
export module handler;

export class Handler {
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

    Handler *next_ = nullptr;
};
