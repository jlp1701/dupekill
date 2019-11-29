//
// Created by Burndi on 18.10.2019.
//

#ifndef DUPEKILL_FILEHASH_H
#define DUPEKILL_FILEHASH_H

#include <string>
#include <memory>
#include "BloomFilterHash.h"

class FileHash {
public:
    FileHash(std::string dirPath, std::string filename, std::unique_ptr<BloomFilterHash> bfHash);
    double compare(const FileHash& otherHash);

    const std::string dirPath = "";
    const std::string filename = "";
    const std::unique_ptr<BloomFilterHash> bfHash = nullptr;
};


#endif //DUPEKILL_FILEHASH_H
