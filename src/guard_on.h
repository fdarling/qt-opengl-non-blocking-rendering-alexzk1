#pragma once
#include <atomic>
#include <mutex>
#include <type_traits>

#define LOCK_GUARD_ON(MUTEX_NAME) std::lock_guard<std::decay<decltype(MUTEX_NAME)>::type> __guard_var##MUTEX_NAME(MUTEX_NAME)

//inside templates use this one macro, it adds "typename"
#define LOCK_GUARD_ON_TEMPL(MUTEX_NAME) std::lock_guard<typename std::decay<decltype(MUTEX_NAME)>::type> __guard_var##MUTEX_NAME(MUTEX_NAME)
