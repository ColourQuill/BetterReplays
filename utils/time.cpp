#include <time.hpp>

// std
#include <iostream>
#include <chrono>
#include <iomanip>

uint64_t Time::seconds(uint64_t seconds, uint64_t milliseconds) {
    return seconds * 1000 + milliseconds;
}
uint64_t Time::minutes(uint64_t minutes, uint64_t seconds, uint64_t milliseconds) {
    return (minutes * 60 + seconds) * 1000 + milliseconds;
}
uint64_t Time::hours(uint64_t hours, uint64_t minutes, uint64_t seconds, uint64_t milliseconds) {
    return ((hours * 60 + minutes) * 60 + seconds) * 1000 + milliseconds;
}

std::string Time::now() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now_c);
#else
    localtime_r(&now_c, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");
    return oss.str();
}