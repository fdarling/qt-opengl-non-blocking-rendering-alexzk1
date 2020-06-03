#pragma once

#include <mutex>

template <class Object, class Mutex>
class LockedObject
{
private:
    mutable Mutex mut;
    Object obj;
public:

    LockedObject() = default;

    void set(const Object& src)
    {
        std::lock_guard<Mutex> grd(mut);
        obj = src;
    }

    Object get() const
    {
        std::lock_guard<Mutex> grd(mut);
        return obj;
    }

    bool setIfDifferent(const Object& src)
    {
        std::lock_guard<Mutex> grd(mut);
        const bool different = !(src == obj);

        if (different)
            obj = src;

        return different;
    }

    bool equal(const Object& c) const
    {
        std::lock_guard<Mutex> grd(mut);
        return obj == c;
    }
};
