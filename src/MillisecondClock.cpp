#include "MillisecondClock.hpp"

void MillisecondClock::start() {
    initialStart = std::chrono::high_resolution_clock::now();
    currentStart = std::chrono::high_resolution_clock::now();
}

long long MillisecondClock::perSecondCheck() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - currentStart);
    return duration.count();
}

long long MillisecondClock::elapsedMilliseconds() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - initialStart);
    return duration.count();
}

void MillisecondClock::reset() {
    currentStart = std::chrono::high_resolution_clock::now();
}