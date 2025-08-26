#pragma once
#ifndef ADDRESS_CACHE_HPP
#define ADDRESS_CACHE_HPP

#include <list>
#include <unordered_map>

template<typename K, typename V>
class AddressCache {
public: 
    explicit AddressCache(size_t capacity) : capacity(capacity) {}

    V& get(const K& key) {
        auto map_it = cache_map.find(key);
        if (map_it == cache_map.end()) {
            throw std::runtime_error("Key not found in cache");
        }
        auto list_it = map_it->second;
        V value = list_it->second;
        cache_list.splice(cache_list.begin(), cache_list, list_it);
        return value;
    }

    void put(const K& key, const V& value) {
        auto map_it = cache_map.find(key);
        if (map_it != cache_map.end()) {
            auto list_it = map_it->second;
            list_it->second = value;
            cache_list.splice(cache_list.begin(), cache_list, list_it);
        } else {
            if (cache_list.size() >= capacity) {
                K last_key = cache_list.back().first;
                cache_list.pop_back();
                cache_map.erase(last_key);
            }
            cache_list.push_front({key, value});
            cache_map[key] = cache_list.begin();
        }
    }

    bool contains(const K& key) const {
        return cache_map.find(key) != cache_map.end();
    }

    size_t size() const {
        return cache_list.size();
    }

    void clear() {
        cache_list.clear();
        cache_map.clear();
    }

private:
    std::list<std::pair<K, V>> cache_list;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> cache_map;
    size_t capacity;
};

#endif