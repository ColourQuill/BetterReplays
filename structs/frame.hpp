#ifndef __FRAME_HPP__
#define __FRAME_HPP__

// better replays
#include <pixel_format.hpp>

// std
#include <vector>
#include <cstdint>

struct Frame {
    std::vector<uint8_t> data;
    PixelFormat pixelFormat;
    int width;
    int height;
    int stride;
    int64_t timestampMS;
};

#endif