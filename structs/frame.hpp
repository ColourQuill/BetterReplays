#ifndef __FRAME_HPP__
#define __FRAME_HPP__

// std
#include <vector>
#include <cstdint>

struct Frame {
    std::vector<uint8_t> data;
    int width;
    int height;
    int stride;
    int64_t timestampMS;
};

#endif