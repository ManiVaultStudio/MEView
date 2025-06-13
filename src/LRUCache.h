#pragma once

#include <list>
#include <unordered_map>
#include <stdexcept>

#include <QHash>

template <typename KeyType, typename ValueType>
class LRUCache
{
private:
    // Define the data structure
    int capacity; // Maximum capacity of the cache
    QList<KeyType> keys; // Doubly linked list to maintain LRU order
    QHash<KeyType, QPair<ValueType, typename QList<KeyType>::iterator>> cache;

public:
    // Constructor
    LRUCache(int cap) : capacity(cap) {}

    // Get an item from the cache
    ValueType get(const KeyType& key) {
        if (!cache.contains(key)) {
            throw std::runtime_error("Key not found in cache");
        }

        // Move the accessed key to the front of the list
        keys.erase(cache[key].second);
        keys.push_front(key);
        cache[key].second = keys.begin();

        return cache[key].first; // Return the value
    }

    // Put an item in the cache
    void put(const KeyType& key, const ValueType& value) {
        if (cache.contains(key)) {
            // If key exists, update value and move to front
            keys.erase(cache[key].second);
        }
        else {
            // If cache is full, evict the least recently used item
            if (keys.size() == capacity) {
                KeyType lru = keys.back(); // LRU key is at the back
                keys.pop_back();
                cache.remove(lru);
            }
        }

        // Insert the new key-value pair
        keys.push_front(key);
        cache[key] = qMakePair(value, keys.begin());
    }
};
