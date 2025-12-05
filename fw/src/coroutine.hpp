/** @file
 * @brief stackless coroutine
 * Plagiarized from asio::coroutine
 * 
 * @{
 */
#pragma once

#include <type_traits>

namespace detail {

template<typename T> class CoroutineRef;

} // namespace detail

template<typename T = int> class Coroutine {
    static_assert(std::is_integral<T>::value && std::is_signed<T>::value, "T must be signed integral type");
public:
    /// Constructs a coroutine in its initial state.
    Coroutine() : value_(0) {}

    /// Returns true if the coroutine is the child of a fork.
    bool is_child() const { return value_ < 0; }

    /// Returns true if the coroutine is the parent of a fork.
    bool is_parent() const { return !is_child(); }

    /// Returns true if the coroutine has reached its terminal state.
    bool is_complete() const { return value_ == -1; }

    T state() const { return value_; }

private:
    friend class detail::CoroutineRef<T>;
    T value_;
};


namespace detail {

template<typename T> class CoroutineRef {
public:
    CoroutineRef(Coroutine<T> &c) : value_(c.value_), modified_(false) {}
    CoroutineRef(Coroutine<T> *c) : value_(c->value_), modified_(false) {}
    ~CoroutineRef() { if (!modified_) value_ = -1; }
    operator T() const { return value_; }
    T &operator=(T v) { modified_ = true; return value_ = v; }
private:
    void operator=(const CoroutineRef&);
    T &value_;
    bool modified_;
};

} // namespace detail

#define CORO_REENTER(c) \
    switch (::detail::CoroutineRef _coro_value = c) \
        case -1: if (_coro_value) { \
            goto terminate_coroutine; \
            terminate_coroutine: \
            _coro_value = -1; \
            goto bail_out_of_coroutine; \
            bail_out_of_coroutine: \
            break; \
        } else /* fall-through */ case 0:

#define CORO_YIELD_IMPL(n) \
  for (_coro_value = (n);;) \
      if (_coro_value == 0) { \
          case (n): ; \
          break; \
      } else \
          switch (_coro_value ? 0 : 1) \
              for (;;) \
                    /* fall-through */ case -1: if (_coro_value) \
                        goto terminate_coroutine; \
                  else for (;;) \
                      /* fall-through */ case 1: if (_coro_value) \
                          goto bail_out_of_coroutine; \
                      else /* fall-through */ case 0:

#define CORO_FORK_IMPL(n) \
    for(_coro_value = -(n);; _coro_value = (n)) \
        if (_coro_value == (n)) { \
            case -(n): ; \
            break; \
        } else

# define CORO_YIELD CORO_YIELD_IMPL(__COUNTER__ + 1)
# define CORO_FORK CORO_FORK_IMPL(__COUNTER__ + 1)

//!@}
