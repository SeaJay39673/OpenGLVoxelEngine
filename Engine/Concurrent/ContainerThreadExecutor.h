#ifndef CONTAINER_THREAD_EXECUTOR
#define CONTAINER_THREAD_EXECUTOR

#include <vector>
#include <thread>
#include <functional>

using std::vector, std::thread, std::function;

namespace Engine::Concurrent
{
    /**
     * @brief A class for executing tasks in parallel using threads.
     *
     * @details This class allows you to execute tasks on a vector or an unordered map in parallel using multiple threads.
     * * The number of threads can be specified during construction or set later using the SetThreads method.
     */
    class ContainerThreadExecutor
    {
    public:
        ContainerThreadExecutor(int t = (int)thread::hardware_concurrency() / 2) : numThreads(t) {}
        void SetThreads(int t)
        {
            numThreads = t;
        }

        template <typename T>
        int Execute(const vector<T> &vec, function<void(T)> task);

        template <typename X, typename T>
        vector<X> Execute(const unordered_map<X, T> &map, function<void(X, T)> task);

    private:
        int numThreads;
        vector<thread> threads;
    };

    /**
     * @brief Executes a task on each element of a vector in parallel using multiple threads.
     *
     * @tparam T The type of the elements in the vector.
     * @param vec The vector of elements to process.
     * @param task The task to execute on each element.
     * @return The number of threads used for execution.
     */
    template <typename T>
    int ContainerThreadExecutor::Execute(const vector<T> &vec, function<void(T)> task)
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

    /**
     * @brief Executes a task on each key-value pair in an unordered map in parallel using multiple threads.
     *
     * @tparam X The type of the keys in the unordered map.
     * @tparam T The type of the values in the unordered map.
     * @param map The unordered map of key-value pairs to process.
     * @param task The task to execute on each key-value pair.
     * @return A vector of keys that were processed.
     */
    template <typename X, typename T>
    vector<X> ContainerThreadExecutor::Execute(const unordered_map<X, T> &map, function<void(X, T)> task)
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

#endif