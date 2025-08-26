#pragma once
#ifndef API_CLIENT_HPP
#define API_CLIENT_HPP

#include <algorithm>
#include <map>
#include <functional>
#include <vector>
#include <string>
#include <utility>

#include "AddressCache.hpp"
#include "dependencies/httplib.h"

class ApiClient {
public:
    explicit ApiClient(const std::string& target);

    enum USE { FETCH_TRANSACTIONS_TRON, FETCH_TRANSACTIONS_SOL, FETCH_TRANSACTIONS_ETH, FETCH_SANCTIONS };

    struct URLs {
        const std::string tron_url = "https://api.trongrid.io";
        const std::string tron_endpoint = "/wallet/getaccount";
        const std::string shyft_url = "https://rpc.shyft.to";
        const std::string shyft_endpoint = "/?api_key=";
        const std::string etherscan_url = "https://api.etherscan.io";
        const std::string chainalysis_url = "https://public.chainalysis.com";
        const std::string chainalysis_endpoint = "/api/v1/address/";
    };

    const static int OK = 200;
    const static int BAD = 400;

    template<USE u>
    std::string sendGETRequest();

    template<USE u>
    std::string sendPOSTRequest();

    httplib::Result getCachedAddressResult(std::string address);

private:
    std::shared_ptr<AddressCache<std::string, httplib::Response>> cache;

    std::shared_ptr<std::vector<std::string>> transaction_addresses;

    std::string target;

    std::string errorToString(httplib::Error err);
};

#endif