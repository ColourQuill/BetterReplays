#include <file.hpp>

#include <filesystem>

std::string File::working() {
    return std::filesystem::current_path().string();
}