#ifndef __SK_UTILITIES_BYTE_INSERTER_H__
#define __SK_UTILITIES_BYTE_INSERTER_H__

#include <deque>
#include <functional>

namespace SK {

class ByteQueue {
private:
    std::reference_wrapper<std::deque<std::byte>> queue_;

public:
    using value_type = std::byte;
    
    ByteQueue(std::deque<std::byte> &queue)
    : queue_{std::ref(queue)} {}

    std::byte get() const {
        return queue_.get().front();
    }

    void pop() {
        queue_.get().pop_front();
    }

    void push(std::byte byte) {
        queue_.get().push_back(byte);
    }
};

} // namespace SK

#endif // __SK_UTILITIES_BYTE_INSERTER_H__
