#include "ApiClient.hpp"
#include <cstdlib>
#include <iomanip>
#include <memory>

const static std::string SANCTIONS = "sanctions";

ApiClient::ApiClient(const std::string& target) : target(target) {
    transaction_addresses = std::make_shared<std::vector<std::string>>();
    cache = std::make_shared<AddressCache<std::string, httplib::Response>>(100);
}

httplib::Result ApiClient::getCachedAddressResult(std::string address) {
    try {
        httplib::Response& cached_response = this->cache->get(address);
        auto res_ptr = std::make_unique<httplib::Response>(std::move(cached_response));
        return httplib::Result(std::move(res_ptr), httplib::Error::Success);
    } catch (...) {
        return httplib::Result(nullptr, httplib::Error::Success);
    }
}

template<ApiClient::USE u>
std::string ApiClient::sendGETRequest() {
    auto eth_handler = [this]() -> std::string {
        auto eth_client = std::make_unique<httplib::Client>(URLs{}.etherscan_url);
        const std::string path = "/api?module=account"
                                 "&action=txlist"
                                 "&address=" + this->target +
                                 "&startblock=0"
                                 "&endblock=99999999"
                                 "&page=1"
                                 "&offset=10"
                                 "&sort=desc"
                                 "&apikey=" + std::getenv("ETHERSCAN_API_KEY");
        auto res = eth_client->Get(path.c_str());
        if (res) {
            if (ApiClient::OK == res->status) {
                std::cout << "ETH API call successful. Received " << res->body.length() << " bytes" << std::endl;
                std::string response = res->body;
                transaction_addresses->clear();
                std::set<std::string> unique_addresses;
                size_t pos = 0;
                while ((pos = response.find("\"to\":", pos)) != std::string::npos) {
                    pos += 5;
                    size_t start = response.find("\"", pos);
                    if (start != std::string::npos) {
                        start++;
                        size_t end = response.find("\"", start);
                        if (end != std::string::npos) {
                            std::string addr = response.substr(start, end - start);
                            if (42 == addr.length() && addr.substr(0, 2) == "0x") {
                                unique_addresses.insert(addr);
                            }
                        }
                    }
                }
                pos = 0;
                while ((pos = response.find("\"from\":", pos)) != std::string::npos) {
                    pos += 7;
                    size_t start = response.find("\"", pos);
                    if (start != std::string::npos) {
                        start++;
                        size_t end = response.find("\"", start);
                        if (end != std::string::npos) {
                            std::string addr = response.substr(start, end - start);
                            if (42 == addr.length() && addr.substr(0, 2) == "0x" && addr != this->target) {
                                unique_addresses.insert(addr);
                            }
                        }
                    }
                }
                for (const std::string& addr : unique_addresses) {
                    transaction_addresses->push_back(addr);
                }
                std::cout << "Extracted " << transaction_addresses->size() << " addresses from transactions" << "\n";
                for (size_t i = 0; i < transaction_addresses->size(); i++) {
                    std::cout << "  Address " << (i+1) << ": " << (*transaction_addresses)[i] << "\n";
                }
                return std::to_string(ApiClient::OK);
            } else {
                std::cout << "ETH API error: " << res->status << "\n";
                return std::to_string(res->status);
            }
        }
        return "Error: " + errorToString(res.error());
    };
    auto sanctions_handler = [this]() -> std::string {
        auto sanctions_client = std::make_unique<httplib::Client>(URLs{}.chainalysis_url);
        httplib::Headers headers = {
                {"X-API-KEY", std::getenv("CHAINALYSIS_API_KEY")},
        };
        std::map<std::string, bool> isAddressSanctioned;
        auto isSanctionedAddress = [this, &sanctions_client]
                (std::string addr, httplib::Headers headers, std::map<std::string, bool>& isAddressSanctioned) -> std::string {
            auto res = sanctions_client->Get(URLs{}.chainalysis_endpoint+addr, headers);
            if (res) {
                if (ApiClient::OK == res->status) {
                    this->cache->put(addr, *res);
                    isAddressSanctioned[addr] = std::string::npos != res->body.find("sanctions");
                    return std::to_string(ApiClient::OK);
                } else {
                    isAddressSanctioned[addr] = false;
                    return std::to_string(res->status);
                }
            } else {
                isAddressSanctioned[addr] = false;
                return "Error: " + errorToString(res.error());
            }
        };
        for (const std::string& addr : *transaction_addresses) {
            std::string result = isSanctionedAddress(addr, headers, isAddressSanctioned);
            std::cout << std::boolalpha;
            std::cout << result << " Sanctioned status: " << static_cast<bool>(isAddressSanctioned[addr]) << "\n";
        }
        return "Sanctions check complete";
    };

    if constexpr (u == ApiClient::USE::FETCH_TRANSACTIONS_ETH) return eth_handler();
    else if constexpr (u == ApiClient::USE::FETCH_SANCTIONS) return sanctions_handler();
    else throw std::runtime_error("Error: Invalid use case for GET.");
}

template<ApiClient::USE u>
std::string ApiClient::sendPOSTRequest() {
    std::map<ApiClient::USE, std::function<std::string()>> post_map;
    post_map[ApiClient::USE::FETCH_TRANSACTIONS_TRON] = [this]() -> std::string {
        auto tron_client = std::make_unique<httplib::Client>(URLs{}.tron_url);
        httplib::Headers headers = {
                {"Content-Type", "application/json"},
                {"TRON-PRO-API-KEY", std::getenv("TRON_API_KEY")},
        };
        std::string body = R"({ "address": ")" + this->target + R"(", "visible": true })";
        auto res = tron_client->Post(URLs{}.tron_endpoint, headers, body, "application/json");
        if (res) {
            if (ApiClient::OK == res->status) return std::to_string(ApiClient::OK);
            else return std::to_string(res->status);
        }
        return "Error: " + errorToString(res.error());
    };
    post_map[ApiClient::USE::FETCH_TRANSACTIONS_SOL] = [this]() -> std::string {
        auto sol_client = std::make_unique<httplib::Client>(URLs{}.shyft_url);
        httplib::Headers headers = {
                {"Content-Type", "application/json"}
        };
        std::string body = R"({
            "jsonrpc": "2.0",
            "id": 1,
            "method": "getAccountInfo",
            "params": [
                ")" + this->target + R"(",
                {
                    "commitment": "finalized",
                    "encoding": "base58"
                }
            ]
        })";
        auto res = sol_client->Post(URLs{}.shyft_endpoint + std::getenv("SHYFT_API_KEY"), headers, body, "application/json");
        if (res) {
            if (ApiClient::OK == res->status) return std::to_string(ApiClient::OK);
            else return std::to_string(res->status);
        }
        return "Error: " + errorToString(res.error());
    };
    
    try {
        return post_map.find(u)->second();
    } catch (...) {
        throw std::runtime_error("Error: Invalid use case for POST.");
    }
}

std::string ApiClient::errorToString(httplib::Error err) {
    switch(err) {
        case httplib::Error::Success:
            return "Success";
        case httplib::Error::Connection:
            return "Connection error";
        case httplib::Error::BindIPAddress:
            return "Bind IP address error";
        case httplib::Error::Read:
            return "Read error";
        case httplib::Error::Write:
            return "Write error";
        case httplib::Error::ExceedRedirectCount:
            return "Exceeded redirect count";
        case httplib::Error::Canceled:
            return "Request canceled";
        case httplib::Error::SSLConnection:
            return "SSL connection error";
        case httplib::Error::SSLLoadingCerts:
            return "SSL loading certificates error";
        case httplib::Error::SSLServerVerification:
            return "SSL server verification error";
        default:
            return "Unknown error";
    }
}

template std::string ApiClient::sendGETRequest<ApiClient::USE::FETCH_TRANSACTIONS_ETH>();
template std::string ApiClient::sendGETRequest<ApiClient::USE::FETCH_SANCTIONS>();
template std::string ApiClient::sendPOSTRequest<ApiClient::USE::FETCH_TRANSACTIONS_TRON>();
template std::string ApiClient::sendPOSTRequest<ApiClient::USE::FETCH_TRANSACTIONS_SOL>();