#ifndef __DISPLAY_RECORDER_HPP__
#define __DISPLAY_RECORDER_HPP__

// better replays
#include <display_config.hpp>

// forward declarations
class ScreenCapture;
class Encoder;
class Buffer;
class Hotkey;

struct DisplayRecorder {
    ScreenCapture* capture = nullptr;
    Encoder* encoder = nullptr;
    Buffer* buffer = nullptr;
    Hotkey* hotkey = nullptr;
    DisplayConfig config;
};

#endif