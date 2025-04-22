#ifndef VECTOR_THREAD_EXECUTOR
#define VECTOR_THREAD_EXECUTOR

#include <vector>
#include <thread>
#include <functional>

using std::vector, std::thread, std::function;

template <typename T>
class VectorThreadExecutor
{
private:
    int numThreads;
    vector<thread> threads;
    function<void(T)> task;

public:
    VectorThreadExecutor(int t = (int)thread::hardware_concurrency() / 2) : numThreads(t) {}
    void SetThreads(int t)
    {
        numThreads = t;
    }

    void AssignTask(function<void(T)> tsk)
    {
        task = tsk;
    }

    int Execute(vector<T> &vec)
    {
        int count = std::min((int)vec.size(), numThreads);
        for (int i = 0; i < count; i++)
        {
            threads.emplace_back(
                [&]()
                {
                    task(vec[i]);
                });
            threads[i].join();
        }
        threads.clear();
        return count;
    }
};

#endif