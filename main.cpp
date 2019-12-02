#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <regex>
#include <fstream>
#include "include/mrshv2/Mrshv2.h"
#include "include/FileHash.h"



int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Wrong number of arguments." << std::endl;
        return -1;
    }
    double simThreshold = std::atoi(argv[1]);
    std::string dirPath(argv[2]);
    std::cout << "Similarity threshold: " << simThreshold << std::endl;
    std::cout << "Search path: " << dirPath << std::endl;
    Mrshv2 mrsh;
    std::regex fileEndRegex(".*\\.bmp$");
    std::vector<std::unique_ptr<FileHash>> fileHashList;
    std::cout << "Start hashing ..." << std::endl;
    for(auto& p: std::filesystem::recursive_directory_iterator(dirPath, std::filesystem::directory_options::skip_permission_denied)) {
        if (!p.is_regular_file() || !std::regex_match(p.path().c_str(), fileEndRegex)) {
            continue;
        }
        std::cout << p.path() << '\n';
        auto fHash = FileHash::generateFileHash(p.path(), mrsh);
        if (fHash) {
            fileHashList.push_back(std::move(fHash));
        } else {
            std::cout << "Failed to generate Hash for file: " << p.path() << std::endl;
        }
    }
    std::cout << "Hashing finished. Hashed " << fileHashList.size() << " files." <<  std::endl;

    std::cout << "Start all-against-all comparison:" << std::endl;
    // make all-against-all comparison
    uint64_t numComp = 0;
    for (uint64_t i=0; i < fileHashList.size(); i++) {
        //std::cout << fileHashList[i].get()->filePath << ": " << std::endl;
        for (uint64_t j=i+1; j < fileHashList.size(); j++) {
            auto sim = fileHashList[i].get()->compare(*fileHashList[j].get());
            numComp++;
            if (sim >= simThreshold){
                std::cout << sim << "%:" << std::endl;
                std::cout << "\t" << fileHashList[i].get()->filePath << std::endl;
                std::cout << "\t" << fileHashList[j].get()->filePath << std::endl;
            }

        }
    }

    // TODO: cluster results according to their similarity
    // TODO: smarter comparisons: for file comparison: only compare when filesizes are similar
    // TODO: Save comparison results in SQLite database
    // TODO: implement filter stages for text-like formats e.g. pdf or doc
    // TODO: How to compare videos and audio files? Maybe use other technique of similarity (other hashing and comparison algorithms for video and audio streams)
    // TODO: implement Levenshtein distance for filenames
    // TODO: implement multi-threading for hashing (depending on bottleneck) and comparing


    std::cout << "Finished comparing. Did " << numComp << " comparations." << std::endl;
    return 0;
}