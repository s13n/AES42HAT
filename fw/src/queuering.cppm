/** @file
 * Ring-based queue
 */

module;
#include <iterator>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <cassert>
export module queuering;


/** Singly linked ring, usable as a queue as well as a stack.
 *
 * This is a variation of a singly linked list (similar to std::forward_list)
 * with a storage-efficient design. It requires a link pointer as a public data
 * member in the object type, i.e. the object type must be a class. Using this
 * link member, the elements in the container are linked into a ring. The last
 * element in the ring points back to the first element. The head points to the
 * last element, so both the last and the first element are easily accessible
 * through the same pointer.
 *
 * In order to access the link embedded in each object, you need to provide a
 * free function next() with the following signature:
 *
 *   QueueRing<T>::pointer &next(QueueRing<T>::const_reference);
 *
 * This function takes a reference to an element, and returns a reference to the
 * link pointer within this element.
 *
 * This way of implementing a list has some advantages and some disadvantages:
 * + Suitable for queue adapter in std library
 * + push_front() is also supported, for stack-like behavior
 * + container can be swapped
 * + No heap allocations, since link pointers are embedded in object
 * + Link from back to front permits rotate() function
 * + Low memory usage; head is one word, overhead per object is also one word
 * +- Container doesn't own its elements, needs external lifetime management
 * - No assignment, copying or moving of the container
 * - No backwards traversal
 * - An iterator needs to hold two pointers to be able to detect end of list
 */
export template<typename T> struct QueueRing {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = ptrdiff_t;
    using reference       = value_type &;
    using const_reference = value_type const &;
    using pointer         = value_type *;
    using const_pointer   = value_type const *;

    /** Iterator into the queue ring.
     *
     * The iterator must be able to indicate a position before the first
     * element. This is achieved by having the pointer cleared to NULL. This,
     * however, requires access to the queue object for advancing to the first
     * element. Hence we hold a pointer to the queue itself, too. The
     * before-the-beginning condition and the past-the-end condition are both
     * encoded with a nullptr. Therefore, a before-the-beginning iterator
     * compares equal to a past-the-end iterator.
     */
    template<bool Const> struct Iter {
        using iterator_category = std::forward_iterator_tag;
        using value_type = QueueRing::value_type;
        using difference_type = QueueRing::difference_type;
        using pointer = std::conditional<Const, QueueRing::const_pointer, QueueRing::pointer>::type;
        using reference = std::conditional<Const, QueueRing::const_reference, QueueRing::reference>::type;

        friend constexpr void swap(Iter &a, Iter &b) noexcept {
            std::swap(a.r_, b.r_);
            std::swap(a.p_, b.p_);
        }
        constexpr reference operator*() noexcept {
            return *p_;
        }
        constexpr pointer operator->() noexcept {
            return p_;
        }
        Iter &operator++() noexcept {
            auto *t = r_->tail_;
            if(p_ == t)
                p_ = nullptr;       // iterator was pointing to last element, or list was empty
            else if(p_)
                p_ = next(*p_);     // iterator was pointing somewhere into the list --> advance
            else
                p_ = next(*t);      // iterator was invalid --> point to first element
            return *this;
        }
        Iter operator++(int) noexcept {
            Iter it{*this};
            ++*this;
            return it;
        }
        constexpr Iter() : r_{nullptr}, p_{nullptr} {}
        constexpr Iter(QueueRing const *r, pointer p) : r_{r}, p_{p} {}
        template<bool C> constexpr Iter(Iter<C> const &it) : r_{it.r_}, p_{it.p_} {}
        constexpr Iter &operator=(Iter it) {
            swap(it, *this);
            return *this;
        }

        QueueRing const *r_;
        pointer p_;
    };
    typedef Iter<false> iterator;
    typedef Iter<true> const_iterator;

    QueueRing(QueueRing const &other) =delete;
    QueueRing &operator=(QueueRing const &other) =delete;

    friend constexpr bool operator==(const_iterator a, const_iterator b) noexcept {
        return a.p_ == b.p_;
    }

    friend constexpr void swap(QueueRing &a, QueueRing &b) noexcept {
        std::swap(a.tail_, b.tail_);
    }
    constexpr QueueRing() : tail_{nullptr} {}
    ~QueueRing() noexcept {
        clear();
    }
    constexpr bool empty() const noexcept {
        return !tail_;
    }
    constexpr iterator begin() noexcept {
        return empty() ? end() : iterator(this, next(*tail_));
    }
    constexpr const_iterator begin() const noexcept {
        return empty() ? end() : const_iterator(this, next(*tail_));
    }
    constexpr const_iterator cbegin() const noexcept {
        return empty() ? end() : const_iterator(this, next(*tail_));
    }
    constexpr iterator end() noexcept {
        return iterator();
    }
    constexpr const_iterator end() const noexcept {
        return const_iterator();
    }
    constexpr const_iterator cend() const noexcept {
        return const_iterator();
    }
    constexpr iterator before_begin() noexcept {
        return iterator(this, nullptr);
    }
    constexpr const_iterator before_begin() const noexcept {
        return const_iterator(this, nullptr);
    }
    constexpr const_iterator cbefore_begin() const noexcept {
        return const_iterator(this, nullptr);
    }
    constexpr reference front() noexcept {
        return *next(*tail_);
    }
    constexpr const_reference front() const noexcept {
        return *next(*tail_);
    }
    constexpr reference back() noexcept {
        return *tail_;
    }
    constexpr const_reference back() const noexcept {
        return *tail_;
    }
    constexpr void rotate() noexcept {                 // move front element to back
        tail_ = next(*tail_);
    }
    constexpr void push_front(reference value) noexcept {
        assert(next(value) == nullptr);
        if(empty()) {
            tail_ = next(value) = &value;
        } else {
            next(value) = next(*tail_);
            next(*tail_) = &value;
        }
    }
    constexpr void pop_front() noexcept {
        auto *p = next(*tail_);
        if(p == tail_)
            tail_ = nullptr;
        else
            next(*tail_) = next(*p);
        next(*p) = nullptr;
    }
    constexpr void push_back(reference value) noexcept {
        push_front(value);
        rotate();
    }
    constexpr iterator erase_after(const_iterator pos) noexcept {
        pointer p = next(pos.p_ ? *pos : *tail_);   // point to object being erased
        if(p != tail_)                              // is this not the tail element?
            return iterator(this, next(*pos) = next(*p));
        tail_ = const_cast<pointer>(pos.p_);        // new tail is the previous element
        if(tail_)
            next(*tail_) = next(*p);
        return iterator(this, nullptr);
    }
    constexpr iterator erase_after(const_iterator first, const_iterator last) noexcept {
        if(first == last)
            return iterator(this, const_cast<pointer>(last.p_));
        iterator it(this, next(*first));
        while(it != last)
            it = erase_after(first);
        return it;
    }
    constexpr void clear() noexcept {
        erase_after(cbefore_begin(), cend());
    }

    pointer tail_;
};
