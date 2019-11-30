#include <iostream>
#include <string>
#include <memory>
#include <filesystem>

#include <fstream>
#include "include/mrshv2/Mrshv2.h"
#include "include/FileHash.h"



int main() {
    Mrshv2 mrsh;

    std::vector<std::unique_ptr<FileHash>> fileHashList;
    for(auto& p: std::filesystem::directory_iterator("bla")) {
        std::cout << p.path() << '\n';
        auto fHash = FileHash::generateFileHash(p.path(), mrsh);
        if (fHash) {
            fileHashList.push_back(std::move(fHash));
        } else {
            std::cout << "Failed to generate Hash for file: " << p.path() << std::endl;
        }
    }

    std::cout << "Start all-against-all comparison:" << std::endl;
    // make all-against-all comparison
    for (uint64_t i=0; i < fileHashList.size(); i++) {
        std::cout << fileHashList[i].get()->filePath << ": " << std::endl;
        for (uint64_t j=i+1; j < fileHashList.size(); j++) {
            auto sim = fileHashList[i].get()->compare(*fileHashList[j].get());
            std::cout << "\t" << sim << "%  File: " << fileHashList[j].get()->filePath << std::endl;
        }
    }

    std::cout << "End" << std::endl;
    return 0;
}