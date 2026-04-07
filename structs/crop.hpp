#ifndef __CROP_HPP__
#define __CROP_HPP__

// std
#include <cstdint>

struct Crop {
    bool valid;
    int x, y;
    uint32_t width, height;
};

#endif