//
// Created by Burndi on 03.10.2019.
//

#ifndef MRSHV2_FNV641A_H
#define MRSHV2_FNV641A_H


#include <cstdint>
#include <vector>

class Fnv641a {
public:
    static uint64_t hash(const std::vector<uint8_t> &data);

private:
    static const uint64_t OffsetBasis = 0xcbf29ce484222325;
    static const uint64_t Prime = 0x00000100000001B3;
};


#endif //MRSHV2_FNV641A_H
