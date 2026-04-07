#ifndef __FORMAT_HPP__
#define __FORMAT_HPP__

// std
#include <cstdint>
#include <vector>

struct FormatInfo {
    uint32_t spaFormat;
    uint32_t drmFormat;
    std::vector<uint64_t> modifiers;
};

#endif