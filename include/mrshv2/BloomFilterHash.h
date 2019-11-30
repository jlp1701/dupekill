//
// Created by Burndi on 04.10.2019.
//

#ifndef MRSHV2_BLOOMFILTERHASH_H
#define MRSHV2_BLOOMFILTERHASH_H

#include <cstdint>
#include <vector>
#include <memory>
#include "BloomFilter.h"

class BloomFilterHash {
public:
    explicit BloomFilterHash(uint32_t m = 2048, uint32_t bfMax = 160);
    BloomFilterHash(const BloomFilterHash&) = delete;
    void operator=(const BloomFilterHash&) = delete;

    void insertHash(uint64_t hashVal);
    double compare(const BloomFilterHash* bfList, bool fragmentCompare = false);
    uint64_t size() const;

private:
    uint32_t m = 0;
    uint32_t bf = 0;
    uint32_t bfMax = 0;
    std::vector<std::unique_ptr<BloomFilter>> bfList;
};


#endif //MRSHV2_BLOOMFILTERHASH_H
