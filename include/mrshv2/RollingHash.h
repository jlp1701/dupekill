//
// Created by Burndi on 03.10.2019.
//

#ifndef MRSHV2_ROLLINGHASH_H
#define MRSHV2_ROLLINGHASH_H

#include <cstdint>
#include <queue>
#include <istream>
#include <vector>
#include <memory>
#include <iostream>

class RollingHash {
public:
    explicit RollingHash(std::istream &data, uint32_t b = 256, uint32_t windowLen = 7);
    void getDataUntilNextTriggerpoint(std::vector<uint8_t> &buffer);
    bool reachedEof();
private:
    std::istream &data;
    uint32_t b = 0;
    uint32_t windowLen = 0;
    uint32_t hashVal = 1;
    const uint32_t modulo = 65521;

    void digestNextByte();
    void insert(uint8_t byte);
    std::vector<uint8_t> dataWindow;
};


#endif //MRSHV2_ROLLINGHASH_H
