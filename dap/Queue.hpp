#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <optional>
#include <vector>

namespace dap
{
template <typename T>
class Queue
{
    std::vector<T> Q;
    std::mutex mutex_lock;
    std::condition_variable cv;

public:
    bool empty() const { return Q.empty(); }

    void push(T o)
    {
        std::unique_lock<std::mutex> locker(mutex_lock);
        Q.emplace_back(o);
        cv.notify_all();
    }

    std::optional<T> pop(const std::chrono::milliseconds& ms)
    {
        std::unique_lock<std::mutex> locker(mutex_lock);
        if (cv.wait_for(locker, ms, [this]() { return !Q.empty(); })) {
            if (Q.empty()) {
                // spuriously wakeup?
                return {};
            }
            // get the first item from the list
            T o = (*Q.begin());
            Q.erase(Q.begin());
            return o;
        }
        return {};
    }
};
}; // namespace dap
#endif // QUEUE_HPP