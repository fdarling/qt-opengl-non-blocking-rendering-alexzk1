#include <QObject>
#include "cpuusage.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <chrono>
#include "strfmt.h"

#ifdef Q_OS_LINUX
    #include <sys/vfs.h>
    #include <sys/types.h>
    #include <unistd.h>

    //USER_HZ detection, from openssl code
    #ifndef HZ
        #if defined(_SC_CLK_TCK) \
            && (!defined(OPENSSL_SYS_VMS) || __CTRL_VER >= 70000000)
            #define HZ ((double)sysconf(_SC_CLK_TCK))
        #else
            #ifndef CLK_TCK
                #ifndef _BSD_CLK_TCK_ /* FreeBSD hack */
                    #define HZ  100.0
                #else /* _BSD_CLK_TCK_ */
                    #define HZ ((double)_BSD_CLK_TCK_)
                #endif
            #else /* CLK_TCK */
                #define HZ ((double)CLK_TCK)
            #endif
        #endif
    #endif
#endif

//from this article: http://cpp.indi.frih.net/blog/2014/09/how-to-read-an-entire-file-into-memory-in-cpp/
template <typename CharT = char,
          typename Traits = std::char_traits<char>>
std::streamsize streamSizeToEnd(std::basic_istream<CharT, Traits>& in)
{
    auto const start_pos = in.tellg();
    if (std::streamsize(-1) == start_pos)
        throw std::ios_base::failure{"error"};

    if (!in.ignore(std::numeric_limits<std::streamsize>::max()))
        throw std::ios_base::failure{"error"};

    const std::streamsize char_count = in.gcount();

    if (!in.seekg(start_pos))
        throw std::ios_base::failure{"error"};

    return char_count;
}

template <typename Container = std::string,
          typename CharT = char,
          typename Traits = std::char_traits<char>>
Container read_stream_into_container(
    std::basic_istream<CharT, Traits>& in,
    typename Container::allocator_type alloc = {})
{
    static_assert(
        // Allow only strings...
        std::is_same<Container, std::basic_string<CharT,
        Traits,
        typename Container::allocator_type>>::value ||
        // ... and vectors of the plain, signed, and
        // unsigned flavours of CharT.
        std::is_same<Container, std::vector<CharT,
        typename Container::allocator_type>>::value ||
        std::is_same<Container, std::vector<
        typename std::make_unsigned<CharT>::type,
        typename Container::allocator_type>>::value ||
        std::is_same<Container, std::vector<
        typename std::make_signed<CharT>::type,
        typename Container::allocator_type>>::value,
        "only strings and vectors of ((un)signed) CharT allowed");

    auto const char_count = streamSizeToEnd(in);

    auto container = Container(std::move(alloc));
    container.resize(char_count);

    if (0 != container.size())
    {
        if (!in.read(reinterpret_cast<CharT*>(&container[0]), container.size()))
            throw std::ios_base::failure{"File size differs"};
    }
    return container;
}


CpuUsage::CpuUsage()
{
}

double CpuUsage::getCpuUsage()
{
    constexpr static double ALPHA = 0.1;

    using namespace std::chrono;
    auto old  = cputime;
    auto oldt = lastCheck;

    lastCheck = Clock::now();
    updateStatFromFs();

    const auto dt = std::max(1l, std::abs(duration_cast<milliseconds>(lastCheck - oldt).count()));

    const double sample = 1.0 * (cputime - old) / dt;

    if (cpu_usage < 0)
        cpu_usage = std::abs(sample);
    else
        cpu_usage = (1.0 - ALPHA) * cpu_usage + ALPHA * sample;

    if (cpu_usage < 0)
        cpu_usage = std::abs(sample);

    return cpu_usage;
}

int CpuUsage::updateStatFromFs()
{
#ifdef Q_OS_LINUX
    std::fstream io(stringfmt("/proc/%d/stat", getpid()), std::ios_base::in );
    std::string fulls;
    try
    {
        fulls = read_stream_into_container(io);
    }
    catch (std::ios_base::failure& e)
    {
    }
    if (fulls.length())
    {
        auto tokens = split(fulls, ' ');
        if (tokens.size() < 14)
            return -1;
        cputime = std::atoi(tokens.at(13).c_str()) * 1000 / HZ;
        cputime += std::atoi(tokens.at(14).c_str()) * 1000 / HZ;
    }
    else
        return -1;
#else
#error "Not Implemented for this OS"
#endif
    return 0;
}
