/** @file
 * Ring-based stack
 */

module;
#include <iterator>
#include <type_traits>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <cassert>
export module stackring;


//! Singly linked ring, usable as a stack.
/** This is a variation of a singly linked list (similar to std::forward_list)
 * with a storage-efficient design. It requires a link pointer named "next_"
 * as a public data member in the object type, i.e. the object type must be
 * a class. Using this link member, the elements in the container are linked
 * into a ring. The last element in the ring points back to the list's head
 * node, which does not count as an element. The head node is always there;
 * it's the StackRing object itself that acts as head node through some type
 * trickery. The StackRing object only holds a single pointer to the first list
 * element, or to itself in case of an empty list.
 *
 * In order to access the link embedded in each object, you need to provide
 * a free function next() with the following signature:
 *
 *   StackRing<T>::pointer &next(StackRing<T>::const_reference);
 *
 * This function takes a reference to an element, and returns a reference to the
 * link pointer within this element.
 *
 * This way of implementing a list has some advantages and some disadvantages:
 * + Suitable for stack adapter in std library
 * + No conditional operations in the list manipulators
 * + No heap allocations, since link pointers are embedded in object
 * + Link back to head permits finding head given an iterator and a range
 * + Low memory usage; head is one word, overhead per object is also one word
 * +- Container doesn't own its elements, needs external lifetime management
 * - No assignment, copying or moving of the container
 * - No backwards traversal
 * - No swap, because list traversal would be needed to implement it
 */
export template<typename T> struct StackRing {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type &;
    using const_reference = value_type const &;
    using pointer         = value_type *;
    using const_pointer   = value_type const *;

    template<bool Const> struct Iter {
        using iterator_category = std::forward_iterator_tag;
        using value_type        = StackRing::value_type;
        using difference_type   = StackRing::difference_type;
        using pointer           = std::conditional<Const, StackRing::const_pointer, StackRing::pointer>::type;
        using reference         = std::conditional<Const, StackRing::const_reference, StackRing::reference>::type;

        ptrdiff_t headpos(StackRing *b) const {
            value_type *v = nullptr;
            void *p = &(next(*v));
            difference_type offset = (char*)p - (char*)v;
            return (StackRing*)((char*)p_ + offset) - b;
        }
        constexpr reference operator*() noexcept {
            return *p_;
        }
        constexpr pointer operator->() noexcept {
            return p_;
        }
        constexpr Iter &operator++() noexcept {
            p_ = next(*p_);
            return *this;
        }
        constexpr Iter operator++(int) noexcept {
            Iter it{*this};
            p_ = next(*p_);
            return it;
        }
        constexpr Iter(pointer p=nullptr) : p_{p} {}

        friend constexpr void swap(Iter &a, Iter &b) noexcept {
            std::swap(a.p_, b.p_);
        }
        friend constexpr bool operator==(Iter const &a, Iter const &b) noexcept {
            return a.p_ == b.p_;
        }

    private:
        pointer p_;
    };
    typedef Iter<false> iterator;
    typedef Iter<true> const_iterator;

    constexpr pointer xlat() const noexcept {
        value_type *v = nullptr;
        void *p = &(next(*v));
        difference_type offset = (char*)p - (char*)v;
        return (pointer)((char*)this - offset);
    }

    StackRing(StackRing const &other) =delete;
    StackRing &operator=(StackRing const &other) =delete;

    constexpr StackRing() : next_(xlat()) {}
    ~StackRing() noexcept {
        clear();
    }
    constexpr iterator begin() noexcept {
        return next_;
    }
    constexpr const_iterator begin() const noexcept {
        return next_;
    }
    constexpr const_iterator cbegin() const noexcept {
        return next_;
    }
    constexpr iterator end() noexcept {
        return xlat();
    }
    constexpr const_iterator end() const noexcept {
        return xlat();
    }
    constexpr const_iterator cend() const noexcept {
        return xlat();
    }
    constexpr iterator before_begin() noexcept {
        return end();
    }
    constexpr const_iterator before_begin() const noexcept {
        return end();
    }
    constexpr const_iterator cbefore_begin() const noexcept {
        return cend();
    }
    constexpr reference back() noexcept {
        return *next_;
    }
    constexpr const_reference back() const noexcept {
        return *next_;
    }
    constexpr void push_back(value_type &value) noexcept {
        next(value) = next_;
        next_ = &value;
    }
    constexpr void pop_back() noexcept {
        next_ = next(*next_);
    }
    constexpr bool empty() const noexcept {
        return next_ == end();
    }
    constexpr iterator erase_after(iterator pos) noexcept {
        return next(*pos) = next(*next(*pos));
    }
    constexpr iterator erase_after(const_iterator pos) noexcept {
        return next(*pos) = next(*next(*pos));
    }
    constexpr iterator erase_after(iterator first, iterator last) noexcept {
        while(next(*first) != last)
            first = erase_after(first);
        return last;
    }
    constexpr iterator erase_after(const_iterator first, const_iterator last) noexcept {
        while(next(*first) != last)
            first = erase_after(first);
        return last;
    }
    constexpr void clear() noexcept {
        erase_after(before_begin(), end());
    }
    template<class UnaryPredicate> constexpr void remove_if(UnaryPredicate p) noexcept {
        auto i1 = before_begin(), i2 = begin();
        while(i2 != end())
            if(p(*i2))
                i2 = erase_after(i1);
            else
                i1 = i2++;
    }

    pointer next_;
};
