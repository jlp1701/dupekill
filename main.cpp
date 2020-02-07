#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <regex>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "include/def.h"
#include "include/FileHash.h"
namespace spd = spdlog;

std::vector<std::unique_ptr<FileHash>> hashFilesInDir(const Mrshv2& mrsh, const std::string& dirPath, const std::string& pattern) {
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
};

struct FileCompResult {
    FileComp comp;
    double similarity = 0;
};

// cluster files according to their size; there can be duplicates of comparisons in clusters
std::vector<FileComp> getComparisons(std::vector<FileHash*>& hList, double relSizeDiff = 0.1) {
    std::vector<FileComp> comparisons = {};

    // for each file get its neighbours
    for (auto it = hList.begin(); it != hList.end(); it++) {
        // get the neighbouring elements
        // forward
        uint64_t minFileSize = (*it)->fileSize * (1.0 - relSizeDiff);
        for (auto itFwd = it; itFwd != hList.end(); itFwd++) {
            if (itFwd == it) continue;
            if ((*itFwd)->fileSize >= minFileSize) {
                FileComp fc;
                fc.files.first = (*it);
                fc.files.second = (*itFwd);
                comparisons.push_back(fc);
            } else {
                // since its a sorted list all following files have smaller sizes
                break;
            }
        }
    }
    return comparisons;
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

void printCompResults(const std::vector<FileCompResult>& compResults, const double th){
    SPDLOG_INFO("Results of comparison: ");
    for (const auto& c : compResults) {
        if (c.similarity >= th) {
            SPDLOG_INFO("file 1 (size: {}) : {}", c.comp.files.first->fileSize, c.comp.files.first->filePath);
            SPDLOG_INFO("file 2 (size: {}) : {}", c.comp.files.second->fileSize, c.comp.files.second->filePath);
            SPDLOG_INFO("\t similarity: {}", c.similarity);
        }
    }
}

int main(int argc, char** argv) {
    spdlog::set_level(spd::level::info);  // needs to be set as well
    spdlog::set_pattern("%v");
    SPDLOG_TRACE("Application starts ...");
    //SPDLOG_DEBUG("Application starts ...");
    //SPDLOG_INFO("Application starts ...");
    //SPDLOG_WARN("Application starts ...");

    double simThreshold = 0.0;
    std::string patternString;
    std::string dirPath;
    double relSizeDiff = 0.0;
    uint64_t logLevelNumber = 0;
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "produce help message")
                ("threshold,t", po::value<double>(&simThreshold)->default_value(0.0), "set the threshold of file similarity. Must be between [0,1.0]")
                ("pattern,p", po::value<std::string>(&patternString)->default_value(".*"), "only selects files which match the regex pattern.")
                ("directory,d", po::value<std::string>(&dirPath), "Path to directory which will be scanned recursively.")
                ("size-diff,s", po::value<double>(&relSizeDiff)->default_value(0.1), "Maximum relative size difference of two files to be compared. Mest be between [0,1].")
                ("log-level,l", po::value<uint64_t>(&logLevelNumber)->default_value(2), "Sets the verbosity of the logging. Range between [0,6]. 0 is most verbose; 6 is no logging.")
                ;

        po::positional_options_description p;
        p.add("directory", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (logLevelNumber > spdlog::level::off) {
            SPDLOG_ERROR("Error: wrong range of logging level: {}", logLevelNumber);
            return 1;
        }
        SPDLOG_INFO("Set log level to: {}", logLevelNumber);
        spdlog::set_level(static_cast<spd::level::level_enum>(logLevelNumber));

        // check argument ranges
        if (simThreshold < 0 || simThreshold > 1.0) {
            SPDLOG_ERROR("Error: wrong range of threshold: {}", simThreshold);
            return 1;
        }
        SPDLOG_INFO("Similarity threshold set to : {}",simThreshold);

        if (relSizeDiff < 0 || relSizeDiff > 1) {
            SPDLOG_ERROR("Error: wrong range of relativ file size difference: {}", relSizeDiff);
            return 1;
        }
        SPDLOG_INFO("Maximum relative file size difference set to: {}", relSizeDiff);

        SPDLOG_INFO("Pattern set to: {}", patternString);
        SPDLOG_INFO("Directory to scan: {}", dirPath);
    }
    catch(std::exception& e) {
        SPDLOG_ERROR("error: {}", e.what());
        return 1;
    }
    catch(...) {
        SPDLOG_ERROR("Exception of unknown type!");
    }

    const Mrshv2 mrsh;

    // get files matching the pattern and hash them
    auto fileHashList = hashFilesInDir(mrsh, dirPath, patternString);

    // sort the list descending according to the file sizes
    std::sort(fileHashList.begin(), fileHashList.end(), [](const auto& f1, const auto& f2) {
            return (f1->fileSize > f2->fileSize);
        });

    // get copy of list
    std::vector<FileHash*> tHashList = {};
    for (const auto& fHash : fileHashList) {
        tHashList.push_back(fHash.get());
    }

    // do clustering of list
    auto comparisons = getComparisons(tHashList, relSizeDiff);
    SPDLOG_INFO("Number of comparisons: {}", comparisons.size());

    auto compResults = compareHashes(comparisons);
    std::sort(compResults.begin(), compResults.end(), [](const auto& r1, const auto& r2) {
        return (r1.similarity > r2.similarity);
    });
    printCompResults(compResults, simThreshold);

    // TODO: Save comparison results in SQLite database
    // TODO: implement filter stages for text-like formats e.g. pdf or doc
    // TODO: How to compare videos and audio files? Maybe use other technique of similarity (other hashing and comparison algorithms for video and audio streams)
    // TODO: implement Levenshtein distance for filenames
    // TODO: implement multi-threading for hashing (depending on bottleneck) and comparing

    SPDLOG_TRACE("Application ends.");
    return 0;
}