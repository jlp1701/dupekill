//
// Created by Burndi on 18.10.2019.
//

#ifndef DUPEKILL_FILEHASH_H
#define DUPEKILL_FILEHASH_H

#include <string>
#include <memory>

#include "def.h"
#include "Mrshv2.h"


class FileHash {
public:
    FileHash(std::string filePath, std::unique_ptr<BloomFilterHash> bfHash);
    double compare(const FileHash& otherHash);

    const std::string filePath = "";
    const std::unique_ptr<BloomFilterHash> bfHash = nullptr;

    static std::unique_ptr<FileHash> generateFileHash(std::string filePath, Mrshv2& hashGen);

};


#endif //DUPEKILL_FILEHASH_H
