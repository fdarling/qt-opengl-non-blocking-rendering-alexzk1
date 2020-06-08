#pragma once
#include "cm_ctors.h"

//maps bind/release to lock/unlock so can be used as mutex :)
template <class Src>
class bind_release_ptr_wrap
{
private:
    Src ptr;
public:
    NO_COPYMOVE(bind_release_ptr_wrap);
    STACK_ONLY;
    bind_release_ptr_wrap() = delete;

    explicit bind_release_ptr_wrap(Src ptr): ptr(ptr) {}

    void lock()
    {
        ptr->bind();
    }

    void unlock()
    {
        ptr->release();
    }
};
