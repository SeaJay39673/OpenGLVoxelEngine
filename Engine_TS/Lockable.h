#ifndef LOCKABLE_H
#define LOCKABLE_H

#include <mutex>

using std::mutex, std::lock_guard;

template <typename T>
struct Lockable
{
    T *ptr = nullptr;
    mutex mut;
};

#endif