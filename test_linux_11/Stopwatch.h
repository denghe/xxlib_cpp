#pragma once

#include <chrono>
#include <stdint.h>

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::time_point;
using std::chrono::hours;
using std::chrono::minutes;
using std::chrono::seconds;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

namespace xxx
{

    class Stopwatch
    {
    public:
        Stopwatch()
            : beginTime( high_resolution_clock::now() )
        {
        }

        inline void Reset()
        {
            beginTime = high_resolution_clock::now();
        }

        inline int64_t ElapsedMillseconds() const
        {
            return duration_cast<milliseconds>( high_resolution_clock::now() - beginTime ).count();
        }

        inline int64_t ElapsedMicroseconds() const
        {
            return duration_cast<microseconds>( high_resolution_clock::now() - beginTime ).count();
        }

        inline int64_t ElapsedNanoseconds() const
        {
            return duration_cast<nanoseconds>( high_resolution_clock::now() - beginTime ).count();
        }

        inline int64_t ElapsedSeconds() const
        {
            return duration_cast<seconds>( high_resolution_clock::now() - beginTime ).count();
        }

        inline int64_t ElapsedMinutes() const
        {
            return duration_cast<minutes>( high_resolution_clock::now() - beginTime ).count();
        }

        inline int64_t ElapsedHours() const
        {
            return duration_cast<hours>( high_resolution_clock::now() - beginTime ).count();
        }

    private:
        time_point<high_resolution_clock> beginTime;
    };

}