#ifndef __TIME_HPP__
#define __TIME_HPP__

// std
#include <cstdint>
#include <string>

struct Time {
    // Returns as milliseconds.
    static uint64_t seconds(uint64_t seconds, uint64_t milliseconds = 0);
    // Returns as milliseconds.
    static uint64_t minutes(uint64_t minutes, uint64_t seconds = 0, uint64_t milliseconds = 0);
    // Returns as milliseconds.
    static uint64_t hours(uint64_t hours, uint64_t minutes = 0, uint64_t seconds = 0, uint64_t milliseconds = 0);

    static std::string now();
};

#endif