#pragma once
#include <functional>
#include <exception>
//simple trickster class - allows to exec function when block ends (i.e. exit from function)

class exec_onexit
{
public:
    using func_t = std::function<void()>;
private:
    func_t func;
public:
    exec_onexit() = delete;
    exec_onexit(const exec_onexit&) = delete;
    exec_onexit(exec_onexit&&) = default;
    exec_onexit& operator =(const exec_onexit&) = delete;
    exec_onexit& operator =(exec_onexit&&) = default;

    exec_onexit(const func_t& func) noexcept:
        func(func) {}

    ~exec_onexit()
    {
        func();
    }
};
