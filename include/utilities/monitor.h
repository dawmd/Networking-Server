#ifndef __SK_UTILITIES_MONITOR_H__
#define __SK_UTILITIES_MONITOR_H__

#include <mutex>

namespace SK {

template<typename T>
class Monitor {
private:
    T object;
    std::mutex mutex;

private:
    class Proxy {
    private:
        Monitor &monitor;
        std::scoped_lock<std::mutex> lock;
    
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
        std::scoped_lock<std::mutex> lock;

    public:
        ConstProxy(const Monitor &monitor_)
        : monitor{monitor_}
        , lock{monitor.lock} {}

        const T &get() const {
            return monitor.object;
        }
    };

public:
    template<typename... Args>
    Monitor(Args &&...args)
    : object(std::forward<Args>(args)...) {}

    Proxy lock() {
        return Proxy{*this};
    }

    ConstProxy lock() const {
        return ConstProxy{*this};
    }
};

} // namespace SK

#endif // __SK_UTILITIES_MONITOR_H__
