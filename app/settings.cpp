#include <settings.hpp>

#include <file.hpp>

bool Settings::load() {
    if (!config.read(File::working() + "/configs/settings.conf")) {
        return false;
    }

    display.resolutionX = config.getInt("Display", "resolutionX");
    display.resolutionY = config.getInt("Display", "resolutionY");
    display.fps = config.getInt("Display", "fps");

    replay.length = config.getInt("Replay", "length");
    replay.format = config.get("Replay", "format");

    return true;
}