//
// Created by Burndi on 18.10.2019.
//

#include "FileHash.h"

FileHash::FileHash(std::string dirPath, std::string filename, std::unique_ptr<BloomFilterHash> bfHash):
    dirPath(std::move(dirPath)), filename(std::move(filename)), bfHash(std::move(bfHash)){};


double FileHash::compare(const FileHash& otherHash){
    return bfHash->compare(otherHash.bfHash.get());
}