#include <iostream>
#include <thread>
#include <vector>

#include "ThreadManager.hpp"

std::atomic<bool> ThreadManager::isProgramActive{true};
std::mutex ThreadManager::consoleMutex;

template<ApiClient::USE u>
void ThreadManager::sendRequest(ApiClient& client, bool verbose, MillisecondClock& clock) {
    try {
        std::string res;
        if constexpr (u == ApiClient::USE::FETCH_TRANSACTIONS_ETH || u == ApiClient::USE::FETCH_SANCTIONS)
            res = client.sendGETRequest<u>();
        else
            res = client.sendPOSTRequest<u>();

        if (verbose) {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "[" << clock.elapsedMilliseconds() << "ms] "
                      << "Request completed: " << res << "\n";
        }

         if constexpr (u == ApiClient::USE::FETCH_SANCTIONS) {
             if (std::string::npos == res.find("Error")) {
                 std::lock_guard<std::mutex> lock(consoleMutex);
                 std::cout << "Sanctions check completed.\n";
             }
         }
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cerr << "Error in request: " << e.what() << "\n";
    }
}

void ThreadManager::runWorkerThread(const std::string& target, const std::string& network, bool verbose) {
    ApiClient client(target);
    MillisecondClock clock;
    clock.start();

    while (isProgramActive.load()) {
        try {
            /* for now, sanctions fetch will only work with eth */
            if (network == "ethereum") {
                sendRequest<ApiClient::USE::FETCH_TRANSACTIONS_ETH>(client, verbose, clock);
                sendRequest<ApiClient::USE::FETCH_SANCTIONS>(client, verbose, clock);
            } else if (network == "tron") {
                sendRequest<ApiClient::USE::FETCH_TRANSACTIONS_TRON>(client, verbose, clock);
                sendRequest<ApiClient::USE::FETCH_SANCTIONS>(client, verbose, clock);
            } else if (network == "solana") {
                sendRequest<ApiClient::USE::FETCH_TRANSACTIONS_SOL>(client, verbose, clock);
                sendRequest<ApiClient::USE::FETCH_SANCTIONS>(client, verbose, clock);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cerr << "Worker thread error: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }
    }
}

void ThreadManager::startMonitoring(const std::string& target, const std::string& network,
                                    int numThreads, bool verbose) {
    std::vector<std::thread> workers;
    std::cout << "Job began with " << numThreads << " threads...\n";
    for (int i = 0; i < numThreads; ++i) {
        workers.emplace_back([=]() {
            runWorkerThread(target, network, verbose);
        });
    }
    std::cout << "Press Enter to stop monitoring...\n";
    std::cin.get();
    isProgramActive.store(false);
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    std::cout << "Monitoring stopped.\n";
}

template void ThreadManager::sendRequest<ApiClient::USE::FETCH_TRANSACTIONS_ETH>(ApiClient&, bool, MillisecondClock&);
template void ThreadManager::sendRequest<ApiClient::USE::FETCH_TRANSACTIONS_TRON>(ApiClient&, bool, MillisecondClock&);
template void ThreadManager::sendRequest<ApiClient::USE::FETCH_TRANSACTIONS_SOL>(ApiClient&, bool, MillisecondClock&);
template void ThreadManager::sendRequest<ApiClient::USE::FETCH_SANCTIONS>(ApiClient&, bool, MillisecondClock&);