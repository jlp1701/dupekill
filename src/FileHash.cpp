//
// Created by Burndi on 18.10.2019.
//

#include <fstream>
#include "../include/FileHash.h"


FileHash::FileHash(std::string filePath, uint64_t fileSize, std::unique_ptr<BloomFilterHash> bfHash):
    filePath(std::move(filePath)), fileSize(fileSize), bfHash(std::move(bfHash)){};


double FileHash::compare(const FileHash& otherHash){
    return bfHash->compare(otherHash.bfHash.get());
}


std::unique_ptr <FileHash> FileHash::generateFileHash(std::string filePath, const Mrshv2& hashGen) {
    // open file as stream
    std::ifstream fStream (filePath, std::ifstream::in | std::ifstream::binary);
    if (!fStream.good()){
        SPDLOG_WARN("Error with opening file: {}", filePath);
        return nullptr;
    }
    // get filesize
    std::streampos fsize = 0;

    fsize = fStream.tellg();
    fStream.seekg( 0, std::ios::end );
    fsize = fStream.tellg() - fsize;

    fStream.seekg(std::ios::beg);

    // compute hash
    auto bfHash = hashGen.computeHash(fStream);

    return std::move(std::make_unique<FileHash>(filePath, static_cast<uint64_t>(fsize), std::move(bfHash)));
}
