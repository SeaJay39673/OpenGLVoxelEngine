#ifndef CONTAINER_THREAD_EXECUTOR
#define CONTAINER_THREAD_EXECUTOR

#include <vector>
#include <thread>
#include <functional>

using std::vector, std::thread, std::function;

namespace Engine::Concurrent
{
    class ContainerThreadExecutor
    {
    private:
        int numThreads;
        vector<thread> threads;

    public:
        ContainerThreadExecutor(int t = (int)thread::hardware_concurrency() / 2) : numThreads(t) {}
        void SetThreads(int t)
        {
            numThreads = t;
        }

        template <typename T>
        int Execute(const vector<T> &vec, function<void(T)> task)
        {
            int count = std::min((int)vec.size(), numThreads);
            for (int i = 0; i < count; i++)
            {
                T value = vec[i];
                threads.emplace_back(
                    [task, value]()
                    {
                        task(value);
                    });
            }
            for (auto &t : threads)
                t.join();
            threads.clear();
            return count;
        }

        template <typename X, typename T>
        vector<X> Execute(const unordered_map<X, T> &map, function<void(X, T)> task)
        {
            int count = 0;
            vector<X> keys;
            for (const auto &pair : map)
            {
                keys.push_back(pair.first);
                threads.emplace_back(
                    [task, pair]()
                    {
                        task(pair.first, pair.second);
                    });
                if (count == numThreads)
                    break;
            }
            for (auto &t : threads)
                t.join();
            threads.clear();
            return keys;
        }
    };
};

#endif