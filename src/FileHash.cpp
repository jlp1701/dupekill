//
// Created by Burndi on 18.10.2019.
//

#include <fstream>
#include "../include/FileHash.h"


FileHash::FileHash(std::string filePath, std::unique_ptr<BloomFilterHash> bfHash):
    filePath(std::move(filePath)), bfHash(std::move(bfHash)){};


double FileHash::compare(const FileHash& otherHash){
    return bfHash->compare(otherHash.bfHash.get());
}


std::unique_ptr <FileHash> FileHash::generateFileHash(std::string filePath, Mrshv2& hashGen) {
    // open file as stream
    std::ifstream fStream (filePath, std::ifstream::in | std::ifstream::binary);
    if (!fStream.good()){
        SPDLOG_WARN("Error with opening file: {}", filePath);
        return nullptr;
    }
    // compute hash
    auto bfHash = hashGen.computeHash(fStream);

    return std::move(std::make_unique<FileHash>(filePath, std::move(bfHash)));
}
