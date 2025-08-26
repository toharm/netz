#pragma once
#ifndef CLI_CLIENT_HPP
#define CLI_CLIENT_HPP

#include <string>
#include <vector>

class CliClient {
public:
    static int parseIntArg(const char* arg, const std::string& flagName);

    static void parseArguments(int argc, char* argv[], int& numThreads, std::string& target, std::string& network);

    static void printBanner(std::string& target, std::string& network, int &numThreads);

    static void displayHelp();

    static bool isValidNetwork(std::string& network);

    static bool isValidAddress(std::string& target, std::string& network);
};

#endif