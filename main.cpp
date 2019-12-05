#include <iostream>
#include <string>
#include <memory>
#include <filesystem>
#include <regex>
#include <fstream>

#include "include/def.h"
#include "include/FileHash.h"
namespace spd = spdlog;

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
    if (argc != 3) {
        SPDLOG_ERROR("Wrong number of arguments: {}", argc);
        return -1;
    }
    double simThreshold = std::atoi(argv[1]);
    std::string dirPath(argv[2]);
    SPDLOG_INFO("Similarity threshold: {}", simThreshold);
    SPDLOG_INFO("Search path: {}", dirPath);
    Mrshv2 mrsh;
    std::regex fileEndRegex(".*\\.bmp$");
    std::vector<std::unique_ptr<FileHash>> fileHashList;
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

    SPDLOG_INFO("Start all-against-all comparison:");
    // make all-against-all comparison
    uint64_t numComp = 0;
    for (uint64_t i=0; i < fileHashList.size(); i++) {
        //std::cout << fileHashList[i].get()->filePath << ": " << std::endl;
        for (uint64_t j=i+1; j < fileHashList.size(); j++) {
            SPDLOG_TRACE("Start comparing file: {} with file: {} ...", fileHashList[i]->filePath, fileHashList[j]->filePath);
            auto sim = fileHashList[i].get()->compare(*fileHashList[j].get());
            SPDLOG_TRACE("    finished. Similarity: {}%", sim);
            numComp++;
            if (sim >= simThreshold){
                SPDLOG_INFO("{}%:", sim);
                SPDLOG_INFO("   {}",fileHashList[i]->filePath);
                SPDLOG_INFO("   {}",fileHashList[j]->filePath);
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
    SPDLOG_WARN("Application ends.");
    return 0;
}