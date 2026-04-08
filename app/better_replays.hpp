#ifndef __BETTER_REPLAYS_HPP__
#define __BETTER_REPLAYS_HPP__

// better replays
#include <settings.hpp>

// forward declaration
class ScreenCapture;
class Encoder;
class Muxer;
class Buffer;
class Hotkey;

class BetterReplays {
    public:
        BetterReplays() {}
        ~BetterReplays();

        bool start();
        bool run();

        bool save();

        void stop();
    private:
        ScreenCapture* capture = nullptr;
        Encoder* encoder = nullptr;
        Muxer* muxer = nullptr;
        Buffer* buffer = nullptr;
        Hotkey* saveHotkey = nullptr;
        Hotkey* exitHotkey = nullptr;

        Settings settings{};
};

#endif