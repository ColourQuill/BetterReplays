#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

// better replays
#include <config.hpp>
#include <display_config.hpp>

// std
#include <vector>

class Settings {
    public:
        Settings() = default;
        ~Settings() = default;

        bool load();
        bool save();

        struct {
            int resolutionX;
            int resolutionY;
            int fps;
        } display;

        struct {
            int length;
            std::string format;
        } replay;

        struct {
            std::string crf;
            std::string preset;
            std::string tune;
            std::string scaling;
        } ffmpeg_encoder;

        std::vector<DisplayConfig> displays;
    private:
        Config config;
};

#endif