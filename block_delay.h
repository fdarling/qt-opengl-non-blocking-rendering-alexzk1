#pragma once
#include <chrono>
#include <stdint.h>
#include <thread>
#include "cm_ctors.h"

/*
 * This class delays block for exact MS set on constructor.
 * */

template <class Delay = std::chrono::milliseconds>
class DelayBlockMs
{
    const std::chrono::steady_clock::time_point sted_b{std::chrono::steady_clock::now()};
    const Delay max;
public:
    NO_COPYMOVE(DelayBlockMs);
    STACK_ONLY;
    DelayBlockMs() = delete;
    explicit DelayBlockMs(Delay max): max(max) {}

    ~DelayBlockMs()
    {
        using namespace std::chrono;
        const auto elapsed = steady_clock::now() - sted_b;
        if (elapsed < max)
            std::this_thread::sleep_for(max - elapsed);
    }
};
