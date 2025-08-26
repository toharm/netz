#include <iostream>
#include <string>
#include <csignal>

#include "CliClient.hpp"
#include "ThreadManager.hpp"

void signalHandler(int signal) {
   std::cout << "\nReceived interrupt signal. Shutting down gracefully..." << std::endl;
   ThreadManager::isProgramActive.store(false);
}

int main(int argc, char* argv[]) {
   std::signal(SIGINT, signalHandler);
   std::signal(SIGTERM, signalHandler);

   std::string target = "";
   std::string network = "ethereum";
   int numThreads = 1;
   bool verbose = false;

   try {
      CliClient::parseArguments(argc, argv, numThreads, target, network);

      for (int i = 1; i < argc; ++i) {
         std::string arg = argv[i];
         if (arg == "--verbose" || arg == "-v") {
            verbose = true;
            break;
         }
      }

      if (target.empty()) {
         std::cerr << "Error: Target address is required. Use --target or -ta flag." << std::endl;
         std::cerr << "Use --help for usage information." << std::endl;
         return EXIT_FAILURE;
      }

      CliClient::printBanner(target, network, numThreads);

      ThreadManager::startMonitoring(target, network, numThreads, verbose);
   } catch (const std::exception& e) {
      std::cerr << "Fatal error: " << e.what() << std::endl;
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}