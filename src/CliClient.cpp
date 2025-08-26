#include <cctype>
#include <iostream>
#include <map>
#include <algorithm>

#include "CliClient.hpp"

int CliClient::parseIntArg(const char* arg, const std::string& flagName) {
    try {
        return std::stoi(arg);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Error: Invalid number for " + flagName + ". Must be a positive integer.");
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Error: Number out of range for " + flagName + ".");
    }
}

bool CliClient::isValidNetwork(std::string& network) {
    std::vector<std::string> valid_networks = {
        "tron",
        "solana",
        "ethereum"
    };
     std::transform(network.begin(), network.end(), network.begin(), ::tolower);
     auto found = std::find(std::begin(valid_networks), std::end(valid_networks), network);
     if (found == valid_networks.end()) throw std::runtime_error("Error: Network " + network + " is not a valid network.\nOptions: tron, solana, ethereum");
     return true;
}

bool CliClient::isValidAddress(std::string& target, std::string& network) {
    std::map<std::string, std::function<bool(std::string&)>> nw_map;
    nw_map["tron"] = [](std::string& target) {
        const size_t tron_size = 34;
        return tron_size == target.length() && std::to_string(target[0]) == "T";
    };
    nw_map["solana"] = [](std::string& target) {
        const size_t min_size = 32;
        const size_t max_size = 44;
        if (min_size > target.length() || max_size < target.length()) return false;
        auto isValidBase58Char = [](char c) {
            const std::string base58_alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
            return base58_alphabet.find(c) != std::string::npos;
        };
        return std::all_of(target.begin(), target.end(), isValidBase58Char);
    };
    nw_map["ethereum"] = [](std::string& target) {
        const size_t eth_size = 42;
        if (target.length() != eth_size) return false;
        if (target.substr(0, 2) != "0x") return false;
        auto isValidHexChar = [](char c) {
            return std::isxdigit(c);
        };
        return std::all_of(target.begin() + 2, target.end(), isValidHexChar);
    };
    auto it = nw_map.find(network);
    if (it == nw_map.end()) throw std::runtime_error("Error: Unknown network " + network);
    if (!it->second(target)) throw std::runtime_error("Error: Invalid address " + target + " for network " + network);
    return true;
}

void CliClient::printBanner(std::string& target, std::string& network, int &numThreads) {
    const char* BLUE  = "\x1b[34m";
    const char* WHITE = "\x1b[37m";
    const char* RESET = "\x1b[0m";

    std::cout << "====================================================" << std::endl;
    std::cout << BLUE <<
    R"(⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⢀⠀⠀⠀⣰⡇⢀⡄⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡄⠀⣿⣰⡀⢠⣿⣇⣾⡇⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣰⣿⣿⢇⣾⣿⣼⣿⢃⡞⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣿⣿⣿⢋⣾⣿⣿⣿⣯⣿⠇⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⢟⣵⣿⣿⣿⣿⣿⣿⣯⡞⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣵⣿⣿⣿⣿⣿⣿⣿⣿⡿⡁⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣦⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠁⠀⠀
)" << RESET;
    std::cout << WHITE <<
    R"(⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⡡⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠁⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⠀⠀⠀
⠀⠀⢀⣀⣄⣀⡀⡀⣠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡥⠀⠀⠀⠀⠀⠀
⠀⣰⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⠀⠀⠀⠀⠀⠀⠀
⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠟⠁⠀⠀⠀⠀⠀⠀⠀
⠘⣿⠋⠛⣿⣿⣿⣿⣿NeTz⣿⣿⣿⣿⣿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣶⣀⡀⠀⠀⠀⠀
)" << RESET;
    std::cout << BLUE <<
    R"(⠀⠀⠀⠀⠘⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⡛⠃⠀⠀
⠀⠀⠀⠀⠀⠀⢈⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀
⠀⠀⠀⠀⠀⢰⣾⣿⣿⣿⣿⣿⠟⠁⠉⠙⠻⠯⡛⠿⠛⠻⠿⠟⠛⠓⠀⠀
⠀⠜⡿⠳⡶⠻⣿⣿⣿⣿⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⣠⣽⣧⣾⠛⠉⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠉⠟⠁⠘⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
)" << RESET << std::endl;

    std::cout << "====================================================" << std::endl;
    std::cout << "Threads:    " << numThreads << std::endl;
    std::cout << "Target:     " << target << std::endl;
    std::cout << "Network:    " << network << std::endl;
    std::cout << "====================================================" << std::endl;
}

void CliClient::parseArguments(int argc, char* argv[], int& numThreads, std::string& target, std::string& network) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        try {
            if (arg == "--help" || arg == "-h") {
                CliClient::displayHelp();
                exit(0);
            } else if (arg == "--threads" || arg == "-th") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Error: --threads requires a value");
                }
                numThreads = CliClient::parseIntArg(argv[++i], "threads");
            } else if (arg == "--network" || arg == "-nw") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Error: --network requires a value");
                }
                network = std::string(argv[++i]);
                CliClient::isValidNetwork(network);
            } else if (arg == "--target" || arg == "-ta") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Error: --target requires a value");
                }
                target = std::string(argv[++i]);
                if (!network.empty()) {
                    CliClient::isValidAddress(target, network);
                }
            } else {
                throw std::runtime_error("Error: Unknown option '" + arg + "'. Use --help for usage.");
            }
        } catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }
    if (!target.empty() && !network.empty()) CliClient::isValidAddress(target, network);
}

void CliClient::displayHelp() {
    std::cout << "Usage: ./program [options] {value}\n"
              << "Options:\n"
              << "  -h, --help                Show this help message\n"
              << "  -th, --threads [num]      Number of threads to run (default: 1)\n"
              << "  -nw, --network [nw]       Blockchain network (tron/solana/ethereum)\n"
              << "  -ta, --target [addr]      Target address to monitor\n"
              << "  -v, --verbose             Enable verbose output\n"
              << "\nExample: \n"
              << "./netz --threads 4 --network ethereum --target 0x123abc...\n";
}