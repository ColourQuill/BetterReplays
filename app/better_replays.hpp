#ifndef __BETTER_REPLAYS_HPP__
#define __BETTER_REPLAYS_HPP__

// better replays
#include <settings.hpp>
#include <display_recorder.hpp>

// forward declaration
class Muxer;
class Hotkey;

class BetterReplays {
    public:
        BetterReplays() {}
        ~BetterReplays();

        bool start();
        bool run();

        bool save(DisplayRecorder& recorder);

        void addDisplay();

        void stop();
    private:
        std::vector<DisplayRecorder> recorders;

        Muxer* muxer = nullptr;
        Hotkey* exitHotkey = nullptr;
        Hotkey* addDisplayHotkey = nullptr;

        Settings settings{};

        bool setupRecorder(DisplayConfig& config);
};

#endif