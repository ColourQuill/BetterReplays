#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

// better replays
#include <config.hpp>

class Settings {
    public:
        Settings() = default;
        ~Settings() = default;

        bool load();

        struct {
            int resolutionX;
            int resolutionY;
            int fps;
        } display;

        struct {
            int length;
            std::string format;
        } replay;
    private:
        Config config;
};

#endif