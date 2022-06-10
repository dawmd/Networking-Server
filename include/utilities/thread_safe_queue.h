#ifndef __SK_UTILITIES_THREAD_SAFE_QUEUE__
#define __SK_UTILITIES_THREAD_SAFE_QUEUE__

#include <mutex>
#include <queue>

namespace SK {

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue{};
    mutable std::mutex mutex{};

public:
    const T &front() const {
        std::scoped_lock<std::mutex> lock{mutex};
        return queue.front();
    }

    const T &back() const {
        std::scoped_lock<std::mutex> lock{mutex};
        return queue.back();
    }

    bool empty() const {
        std::scoped_lock<std::mutex> lock{mutex};
        return queue.empty();
    }

    std::size_t size() const {
        std::scoped_lock<std::mutex> lock{mutex};
        return queue.size();
    }

    void push(const T &element) {
        std::scoped_lock<std::mutex> lock{mutex};
        queue.push(element);
    }

    void push(T &&element) {
        std::scoped_lock<std::mutex> lock{mutex};
        queue.push(std::move(element));
    }

    template<typename... Args>
    decltype(auto) emplace(Args &&...args) {
        std::scoped_lock<std::mutex> lock{mutex};
        queue.emplace(std::forward<Args>(args)...);
    }

    void pop() {
        std::scoped_lock<std::mutex> lock{mutex};
        queue.pop();
    }

    T move_out() {
        std::scoped_lock<std::mutex> lock{mutex};
        auto result = std::move(queue.front());
        queue.pop();
        return result;
    }
};

} // namespace SK

#endif // __SK_UTILITIES_THREAD_SAFE_QUEUE__
