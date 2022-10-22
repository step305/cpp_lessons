//
// Created by sanch on 22.10.2022.
//

#ifndef SIMPLE_CIRCULAR_FIFO_H
#define SIMPLE_CIRCULAR_FIFO_H
/*
 * Not any company's property but Public-Domain
 * Do with source-code as you will. No requirement to keep this
 * header if need to use it/change it/ or do whatever with it
 *
 * Note that there is No guarantee that this code will work
 * and I take no responsibility for this code and any problems you
 * might get if using it.
 *
 * Code & platform dependent issues with it was originally
 * published at http://www.kjellkod.cc/threadsafecircularqueue
 * 2012-16-19  @author Kjell Hedström, hedstrom@kjellkod.cc */

#include <atomic>
#include <cstddef>
#include <mutex>
#include <condition_variable>

namespace circ_fifo {
    template<typename Element, size_t Size>
    class CircularFifo{
    public:
        enum { Capacity = Size+1 };

        CircularFifo(bool blocking_) : _tail(0), _head(0), blocking{blocking_} {}
        virtual ~CircularFifo() {}

        bool push(const Element& item);
        bool pop(Element& item);

        bool wasEmpty() const;
        bool wasFull() const;
        bool isLockFree() const;

        void notify();

    private:
        size_t increment(size_t idx) const;

        std::atomic <size_t>  _tail;  // tail(input) index
        Element    _array[Capacity];
        std::atomic<size_t>   _head; // head(output) index
        std::mutex mutex_;
        std::condition_variable cond_;
        bool blocking;
    };

// Here with memory_order_seq_cst for every operation. This is overkill but easy to reason about
//
// Push on tail. TailHead is only changed by producer and can be safely loaded using memory_order_relexed
//         head is updated by consumer and must be loaded using at least memory_order_acquire
    template<typename Element, size_t Size>
    bool CircularFifo<Element, Size>::push(const Element& item)
    {
        if (blocking) {
            std::unique_lock<std::mutex> mlock(mutex_);
            const auto current_tail = _tail.load();
            const auto next_tail = increment(current_tail);
            if(next_tail != _head.load()) {
                _array[current_tail] = item;
                _tail.store(next_tail);
                cond_.notify_one();
                return true;
            }
            return false;  // full queue
        } else {
            const auto current_tail = _tail.load();
            const auto next_tail = increment(current_tail);
            if(next_tail != _head.load()) {
                _array[current_tail] = item;
                _tail.store(next_tail);
                return true;
            }
            return false;
        }
    }

// Pop by Consumer can only update the head
    template<typename Element, size_t Size>
    bool CircularFifo<Element, Size>::pop(Element& item)
    {
        if (blocking) {
            std::unique_lock<std::mutex> mlock(mutex_);
            const auto current_head = _head.load();
            while (current_head == _tail.load())
                cond_.wait(mlock);
            item = _array[current_head];
            _head.store(increment(current_head));
            return true;
        } else {
            const auto current_head = _head.load();
            if(current_head == _tail.load())
                return false;   // empty queue
            item = _array[current_head];
            _head.store(increment(current_head));
            return true;
        }

    }

// snapshot with acceptance of that this comparison function is not atomic
// (*) Used by clients or test, since pop() avoid double load overhead by not
// using wasEmpty()
    template<typename Element, size_t Size>
    bool CircularFifo<Element, Size>::wasEmpty() const
    {
        return (_head.load() == _tail.load());
    }

// snapshot with acceptance that this comparison is not atomic
// (*) Used by clients or test, since push() avoid double load overhead by not
// using wasFull()
    template<typename Element, size_t Size>
    bool CircularFifo<Element, Size>::wasFull() const
    {
        const auto next_tail = increment(_tail.load());
        return (next_tail == _head.load());
    }


    template<typename Element, size_t Size>
    bool CircularFifo<Element, Size>::isLockFree() const
    {
        return (_tail.is_lock_free() && _head.is_lock_free());
    }

    template<typename Element, size_t Size>
    size_t CircularFifo<Element, Size>::increment(size_t idx) const
    {
        return (idx + 1) % Capacity;
    }

    template<typename Element, size_t Size>
    void CircularFifo<Element, Size>::notify()
    {
        cond_.notify_all();
    }

} // sequential_consistent

#endif //SIMPLE_CIRCULAR_FIFO_H
