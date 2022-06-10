#ifndef __SK_UTILITIES_MONITOR_H__
#define __SK_UTILITIES_MONITOR_H__

#include <mutex>

namespace SK {

template<typename T>
class Monitor {
private:
    T object;
    mutable std::mutex mutex;

private:
    class Proxy {
    private:
        Monitor &monitor;
        std::lock_guard<std::mutex> lock;
    
    public:
        Proxy(Monitor &monitor_)
        : monitor{monitor_}
        , lock{monitor.mutex} {}

        T &get() {
            return monitor.object;
        }
    };

    class ConstProxy {
    private:
        const Monitor &monitor;
        std::lock_guard<std::mutex> lock;

    public:
        ConstProxy(const Monitor &monitor_, std::mutex &mutex)
        : monitor{monitor_}
        , lock{mutex} {}

        const T &get() const {
            return monitor.object;
        }
    };

public:
    template<typename... Args>
    Monitor(Args &&...args)
    : object(std::forward<Args>(args)...) {}

    Monitor(Monitor &&monitor)
    : object{std::move(monitor.object)}
    , mutex{} {}

    Proxy lock() {
        return Proxy{*this};
    }

    ConstProxy lock() const {
        return ConstProxy{*this, this->mutex};
    }
};

} // namespace SK

#endif // __SK_UTILITIES_MONITOR_H__
