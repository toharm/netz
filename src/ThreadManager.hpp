#pragma once

#ifndef THREAD_MANAGER_HPP
#define THREAD_MANAGER_HPP

#include <atomic>
#include <mutex>
#include <string>

#include "ApiClient.hpp"
#include "MillisecondClock.hpp"

class MillisecondClock;

class ThreadManager {
public:
    static std::atomic<bool> isProgramActive;
    static std::mutex consoleMutex;

    template<ApiClient::USE u>
    static void sendRequest(ApiClient& client, bool verbose, MillisecondClock& clock);

    static void runWorkerThread(const std::string& target, const std::string& network, bool verbose);

    static void startMonitoring(const std::string& target, const std::string& network,
                                int numThreads, bool verbose);
};

#endif