#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <regex>
#include <fstream>
#include <unordered_set>

#include "include/def.h"
#include "include/FileHash.h"
namespace spd = spdlog;

std::vector<std::unique_ptr<FileHash>> hashFilesInDir(const Mrshv2& mrsh, const std::string dirPath, const std::string pattern = ".*") {
    std::regex fileEndRegex(pattern);
    std::vector<std::unique_ptr<FileHash>> fileHashList = {};
    SPDLOG_INFO("Start hashing ...");
    for(auto& p: std::filesystem::recursive_directory_iterator(dirPath, std::filesystem::directory_options::skip_permission_denied)) {
        if (!p.is_regular_file() || !std::regex_match(p.path().c_str(), fileEndRegex)) {
            continue;
        }
        SPDLOG_DEBUG("{}", p.path().c_str());
        auto fHash = FileHash::generateFileHash(p.path(), mrsh);
        if (fHash) {
            fileHashList.push_back(std::move(fHash));
        } else {
            SPDLOG_WARN("Failed to generate Hash for file: {}", p.path().c_str());
        }
    }
    SPDLOG_INFO("Hashing finished. Hashed {} files.", fileHashList.size());
    return fileHashList;
}

struct FileComp {
    std::pair<FileHash*, FileHash*> files;

    bool operator==(const FileComp &fc) const {
        return (this->files.first == fc.files.first && this->files.second == fc.files.second) ||
               (this->files.first == fc.files.second && this->files.second == fc.files.first);
    }

};

//bool compFileComp(const FileComp& fc1, const FileComp& fc2) {
//    return (fc1.files.first == fc2.files.first && fc1.files.second == fc2.files.second) ||
//           (fc1.files.first == fc2.files.second && fc1.files.second == fc2.files.first);
//}

//size_t hashFileComp(const FileComp& fc) {
//    std::hash<std::string> strHash;
//    return strHash(fc.files.first->filePath);
//}
struct hashFileComp {
    size_t operator()(const FileComp &fc) const {
        return std::hash<std::string>()(fc.files.first->filePath) ^ std::hash<std::string>()(fc.files.second->filePath);
    }
};

struct FileCompResult {
    FileComp comp;
    double similarity = 0;
};

// cluster files according to their size; there can be duplicates of comparisons in clusters
std::vector<FileComp> getComparisons(std::vector<FileHash*>& hList, double relSizeDiff = 0.1) {
    std::unordered_set<FileComp, hashFileComp> comparisons = {};

    // for each file get its neighbours
    for (auto it = hList.begin(); it != hList.end(); it++) {
        // get the neighbouring elements
        // forward
        for (auto itFwd = it; itFwd != hList.end(); itFwd++) {
            if (itFwd == it) continue;
            if ((*it)->fileSize + (*it)->fileSize*relSizeDiff >= (*itFwd)->fileSize) {
                FileComp fc;
                fc.files.first = (*it);
                fc.files.second = (*itFwd);
                comparisons.insert(fc);
            } else {
                // since its a sorted list all following files have bigger sizes
                break;
            }
        }
        // backward
        for (auto itRwd = it; itRwd != hList.begin(); itRwd--) {
            if (itRwd == it) continue;
            SPDLOG_DEBUG("f1 size: {}", (*it)->fileSize);
            SPDLOG_DEBUG("f2 size: {}", (*itRwd)->fileSize);
            if ((*it)->fileSize - (*it)->fileSize*relSizeDiff <= (*itRwd)->fileSize) {
                FileComp fc;
                fc.files.first = (*it);
                fc.files.second = (*itRwd);
                comparisons.insert(fc);
            } else {
                // since its a sorted list all following files have bigger sizes
                break;
            }
        }
    }
    std::vector<FileComp> v = {};
    std::copy(comparisons.begin(), comparisons.end(), std::back_inserter(v));
    return v;
}


// do the comparisons and store the results (= similarity) in a list
std::vector<FileCompResult> compareHashes(const std::vector<FileComp>& fCompList){
    std::vector<FileCompResult> comResults = {};

    for (const auto& fc : fCompList) {
        FileCompResult fcRes;
        fcRes.comp = fc;
        fcRes.similarity = fc.files.first->compare(*fc.files.second);
        comResults.push_back(fcRes);
    }
    return comResults;
}

void printCompResults(const std::vector<FileCompResult>& compResults){
    std::cout << "Results of comparison: " << std::endl;
    for (const auto& c : compResults) {
        std::cout << "file 1 (size: " << c.comp.files.first->fileSize << " ): " << c.comp.files.first->filePath << std::endl;
        std::cout << "file 2 (size: " << c.comp.files.second->fileSize << " : " << c.comp.files.second->filePath << std::endl;
        std::cout << "\t similarity: " << c.similarity << std::endl;
    }
}

int main(int argc, char** argv) {
//    auto console_sink = std::make_shared<spd::sinks::stdout_color_sink_mt>();
//    console_sink->set_level(spd::level::trace);
//    auto file_sink = std::make_shared<spd::sinks::basic_file_sink_mt>("logs/multisink.txt", true);
//    file_sink->set_level(spd::level::trace);
//    spd::sinks_init_list sInitList({console_sink, file_sink});
//    auto shLogger = std::make_shared<spd::logger>("main", sInitList);
//    spd::set_default_logger(shLogger);

    //spd::logger& logger = *shLogger;
    //logger.set_level(spd::level::debug);
    //spd::warn("this should appear in both console and file");
    //spd::info("this message should not appear in the console, only in the file");
    spdlog::set_level(spd::level::trace);  // needs to be set as well
    SPDLOG_TRACE("Application starts ...");
    SPDLOG_DEBUG("Application starts ...");
    SPDLOG_INFO("Application starts ...");
    SPDLOG_WARN("Application starts ...");

    // check arguments
    if (argc != 3) {
        SPDLOG_ERROR("Wrong number of arguments: {}", argc);
        return -1;
    }
    double simThreshold = std::atoi(argv[1]);
    std::string dirPath(argv[2]);
    SPDLOG_INFO("Similarity threshold: {}", simThreshold);
    SPDLOG_INFO("Search path: {}", dirPath);
    const Mrshv2 mrsh;

    // get files matching the pattern and hash them
    auto fileHashList = hashFilesInDir(mrsh, dirPath, ".*\\.txt");

    // sort the list according to the file sizes
    std::sort(fileHashList.begin(), fileHashList.end(), [](const auto& f1, const auto& f2) {
            return (f1->fileSize < f2->fileSize);
        });

    // get copy of list
    std::vector<FileHash*> tHashList = {};
    for (const auto& fHash : fileHashList) {
        tHashList.push_back(fHash.get());
    }

    // do clustering of list
    auto comparisons = getComparisons(tHashList);
    SPDLOG_INFO("Number of comparisons: {}", comparisons.size());

    auto compResults = compareHashes(comparisons);
    std::sort(compResults.begin(), compResults.end(), [](const auto& r1, const auto& r2) {
        return (r1.similarity > r2.similarity);
    });
    printCompResults(compResults);

//    // compare the hashes and check for similar files
//    SPDLOG_INFO("Start all-against-all comparison:");
//    // make all-against-all comparison
//    uint64_t numComp = 0;
//    for (uint64_t i=0; i < fileHashList.size(); i++) {
//        //std::cout << fileHashList[i].get()->filePath << ": " << std::endl;
//        for (uint64_t j=i+1; j < fileHashList.size(); j++) {
//            SPDLOG_TRACE("Start comparing file: {} with file: {} ...", fileHashList[i]->filePath, fileHashList[j]->filePath);
//            auto sim = fileHashList[i].get()->compare(*fileHashList[j].get());
//            SPDLOG_TRACE("    finished. Similarity: {}%", sim);
//            numComp++;
//            if (sim >= simThreshold){
//                SPDLOG_INFO("{}%:", sim);
//                SPDLOG_INFO("   {}",fileHashList[i]->filePath);
//                SPDLOG_INFO("   {}",fileHashList[j]->filePath);
//            }
//
//        }
//    }
//    std::cout << "Finished comparing. Did " << numComp << " comparations." << std::endl;


    // CLI:
    //  positional argument:
    //      dir: marks the path location where to search for files
    //  optional arguments:
    //      - t: define a threshold of similarity in percentage (default: 90%)
    //      - p: defines a pattern to match files in the directory (default: .*)
    //      - v: more output (could be difficult because we use macros for logging)
    //      - h: print help
    // TODO: cluster results according to their similarity
    // TODO: smarter comparisons: for file comparison: only compare when filesizes are similar
    // TODO: Save comparison results in SQLite database
    // TODO: implement filter stages for text-like formats e.g. pdf or doc
    // TODO: How to compare videos and audio files? Maybe use other technique of similarity (other hashing and comparison algorithms for video and audio streams)
    // TODO: implement Levenshtein distance for filenames
    // TODO: implement multi-threading for hashing (depending on bottleneck) and comparing

    SPDLOG_WARN("Application ends.");
    return 0;
}