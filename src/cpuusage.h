#pragma once
#include <stdint.h>
#include <chrono>

class CpuUsage
{
public:
    CpuUsage();
    double getCpuUsage();
private:
    using Clock = std::chrono::high_resolution_clock;
    int64_t cputime{0};
    double cpu_usage{0.};
    Clock::time_point lastCheck;
    int updateStatFromFs();
};
