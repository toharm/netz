#pragma once
#ifndef MILLISECOND_CLOCK_HPP
#define MILLISECOND_CLOCK_HPP

#include <chrono>

class MillisecondClock {
public:
    // start clock
    void start();

    // elapsed time in milliseconds since current start
    long long perSecondCheck();

    // elapsed time in milliseconds since initial start
    long long elapsedMilliseconds();

    // reset clock
    void reset();
private:
    std::chrono::high_resolution_clock::time_point initialStart; // store start time
    std::chrono::high_resolution_clock::time_point currentStart;
};

#endif