#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <vector>

using namespace std;
namespace dap
{
template <typename T>
class Queue
{
    vector<T> Q;
    mutex mutex_lock;
    condition_variable cv;

public:
    bool empty() const { return Q.empty(); }

    void push(T o)
    {
        unique_lock<mutex> locker(mutex_lock);
        Q.emplace_back(o);
        cv.notify_all();
    }

    T pop(const chrono::milliseconds& ms)
    {
        unique_lock<mutex> locker(mutex_lock);
        if(cv.wait_for(locker, ms, [this]() { return !Q.empty(); })) {
            if(Q.empty()) {
                // spuriously wakeup?
                return T();
            }
            // get the first item from the list
            T o = (*Q.begin());
            Q.erase(Q.begin());
            return o;
        }
        return T();
    }
};
};     // namespace dap
#endif // QUEUE_HPP