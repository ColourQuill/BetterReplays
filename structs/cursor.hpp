#ifndef __CURSOR_HPP__
#define __CURSOR_HPP__

struct Cursor {
    bool visible;
    bool valid;
    int x, y;
    int hotspot_x, hotspot_y;
    int width, height;
};

#endif