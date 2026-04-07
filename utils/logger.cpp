#include <logger.hpp>

// std
#include <iostream>
#include <chrono>
#include <iomanip>

void Logger::logInfo(const std::string& str) {
    std::cout << getTime() << " INFO: " << str << std::endl;
}
void Logger::logError(const std::string& str) {
    std::cerr << getTime() << " ERROR: " << str << std::endl;
}
void Logger::logWarning(const std::string& str) {
    std::cout << getTime() << " WARNING: " << str << std::endl;
}
void Logger::logInfo(const std::string& source, const std::string& contents) {
    std::cout << getTime() << " [" << source << "] INFO: " << contents << std::endl;
}
void Logger::logError(const std::string& source, const std::string& contents) {
    std::cerr << getTime() << " [" << source << "] ERROR: " << contents << std::endl;
}
void Logger::logWarning(const std::string& source, const std::string& contents) {
    std::cout << getTime() << " [" << source << "] WARNING: " << contents << std::endl;
}

std::string Logger::ptrToStr(const void* p) {
    std::ostringstream oss;
    oss << p;
    return oss.str();
}

std::string Logger::getTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now_c);
#else
    localtime_r(&now_c, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
