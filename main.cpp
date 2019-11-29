#include <iostream>
#include <string>
#include <memory>
#include <experimental/filesystem>
#include "Mrshv2.h"
#include "FileHash.h"

int main() {
    Mrshv2 mrsh;
    std::string dir = "1";
    std::string fName = "2";
    std::unique_ptr<BloomFilterHash> bfHash = std::make_unique<BloomFilterHash>();
    FileHash fh(dir, fName, std::move(bfHash));
    dir = "s";
    std::cout << "Hello, World!" << std::endl;
    return 0;
}