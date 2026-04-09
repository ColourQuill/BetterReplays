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

    ffmpeg_encoder.crf = config.get("FFmpeg-Encoder", "crf");
    ffmpeg_encoder.preset = config.get("FFmpeg-Encoder", "preset");
    ffmpeg_encoder.tune = config.get("FFmpeg-Encoder", "tune");
    ffmpeg_encoder.scaling = config.get("FFmpeg-Encoder", "scaling");

    int i = 1;
    while (true) {
        std::string section = "Display-" + std::to_string(i++);
        std::string name = config.get(section, "name");
        if (name.empty()) break;

        DisplayConfig display;
        display.name         = name;
        display.restoreToken = config.get(section, "restoreToken");
        display.saveHotkeyA  = config.getInt(section, "saveHotkeyA");
        display.saveHotkeyB  = config.getInt(section, "saveHotkeyB");
        displays.push_back(display);
    }

    return true;
}

bool Settings::save() {
    config.set("Display", "resolutionX", std::to_string(display.resolutionX));
    config.set("Display", "resolutionY", std::to_string(display.resolutionY));
    config.set("Display", "fps",         std::to_string(display.fps));

    config.set("Replay", "length", std::to_string(replay.length));
    config.set("Replay", "format", replay.format);

    config.set("FFmpeg-Encoder", "crf",     ffmpeg_encoder.crf);
    config.set("FFmpeg-Encoder", "preset",  ffmpeg_encoder.preset);
    config.set("FFmpeg-Encoder", "tune",    ffmpeg_encoder.tune);
    config.set("FFmpeg-Encoder", "scaling", ffmpeg_encoder.scaling);

    for (int i = 0; i < (int)displays.size(); i++) {
        std::string section      = "Display-" + std::to_string(i + 1);
        const DisplayConfig& display = displays[i];

        config.set(section, "name",         display.name);
        config.set(section, "restoreToken", display.restoreToken);
        config.set(section, "saveHotkeyA",  std::to_string(display.saveHotkeyA));
        config.set(section, "saveHotkeyB",  std::to_string(display.saveHotkeyB));
    }

    return config.overwrite(File::working() + "/configs/settings.conf");
}