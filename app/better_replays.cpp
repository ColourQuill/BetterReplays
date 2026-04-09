#include <better_replays.hpp>

// better replays
#include <screen_capture.hpp>
#include <encoder.hpp>
#include <muxer.hpp>
#include <buffer.hpp>
#include <hotkey.hpp>
#include <time.hpp>
#include <logger.hpp>
#include <input.hpp>
#include <portal.hpp>

// std
#include <thread>
#include <chrono>
#include <algorithm>

BetterReplays::~BetterReplays() {
    for (auto& recorder : recorders) {
        delete recorder.capture;
        delete recorder.encoder;
        delete recorder.buffer;
        delete recorder.hotkey;
    }
    recorders.clear();

    delete muxer;
    delete exitHotkey;

    muxer = nullptr;
    exitHotkey = nullptr;
}

bool BetterReplays::start() {
    if (!settings.load()) {
        Logger::logError("BetterReplays", "Failed to load settings.");
        return false;
    }

    muxer = Muxer::create();
    muxer->setSettings(&settings);
    exitHotkey = Hotkey::create();
    addDisplayHotkey = Hotkey::create();

    recorders.reserve(settings.displays.size());
    for (auto& config : settings.displays) {
        if (!setupRecorder(config)) return false;
    }

    return true;

}

bool BetterReplays::run() {
    addDisplayHotkey->start(Key::KEY_left_alt, Key::KEY_slash, [this]() { addDisplay(); });
    exitHotkey->start(Key::KEY_left_alt, Key::KEY_comma, [&]() { stop(); });
    Logger::logInfo("BetterReplays", "Running - press (Alt + ,) to exit.");

    while (true) {
        if (!recorders.empty() && std::none_of(recorders.begin(), recorders.end(), [](const DisplayRecorder& r) {
            return r.capture->isRunning();
        })) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    exitHotkey->stop();
    for (auto& recorder : recorders) {
        recorder.hotkey->stop();
    }

    return true;

}

bool BetterReplays::save(DisplayRecorder& recorder) {
    std::string filename = "Replay " + Time::now() + "." + settings.replay.format;
    Logger::logInfo("BetterReplays", "Saving replay for " + recorder.config.name + "...");

    std::vector<EncodedPacket> snapshot = recorder.buffer->getSnapshot();
    if (!muxer->save(filename, snapshot, recorder.encoder->getCodecParameters())) {
        Logger::logError("BetterReplays", "Failed to save replay for " + recorder.config.name);
        return false;
    }

    Logger::logInfo("BetterReplays", "Saved -> " + filename);
    return true;
}

void BetterReplays::stop() {
    for (auto& recorder : recorders) {
        recorder.capture->stop();
    }
    Logger::logInfo("BetterReplays", "Shutdown complete.");
}

bool BetterReplays::setupRecorder(DisplayConfig& config) {
    recorders.reserve(recorders.size() + 1);

    DisplayRecorder recorder;
    recorder.config  = config;
    recorder.capture = ScreenCapture::create();
    recorder.encoder = Encoder::create();
    recorder.buffer  = new Buffer(Time::seconds(settings.replay.length));
    recorder.hotkey  = Hotkey::create();

    recorder.capture->setSettings(&settings);
    recorder.encoder->setSettings(&settings);
    recorder.capture->setDisplayConfig(config);

    recorders.push_back(std::move(recorder));

    size_t idx = recorders.size() - 1;

    recorders[idx].encoder->setPacketCallback([this, idx](const EncodedPacket& packet) {
        recorders[idx].buffer->addPacket(packet);
    });
    recorders[idx].capture->setFrameCallback([this, idx](const Frame& frame) {
        recorders[idx].encoder->encode(frame);
    });
    recorders[idx].capture->setEncoderInitCallback([this, idx](uint32_t w, uint32_t h, PixelFormat fmt) {
        if (!recorders[idx].encoder->init(w, h, fmt, settings.display.resolutionX, settings.display.resolutionY, settings.display.fps)) {
            Logger::logError("BetterReplays", "Failed to initialize encoder for " + recorders[idx].config.name);
        }
    });

    if (!recorders[idx].capture->init()) {
        Logger::logError("BetterReplays", "Failed to initialize capture for " + recorders[idx].config.name);
        return false;
    }

    if (!recorders[idx].capture->start()) {
        Logger::logError("BetterReplays", "Failed to start capture for " + recorders[idx].config.name);
        return false;
    }

    recorders[idx].hotkey->start(config.saveHotkeyA, config.saveHotkeyB, [this, idx]() {
        save(recorders[idx]);
    });

    Logger::logInfo("BetterReplays", "Recording " + config.name);
    return true;
}

void BetterReplays::addDisplay() {
    std::thread([this]() {
        int index = recorders.size() + 1;
        DisplayConfig config = Portal::selectDisplay(index);
        if (config.restoreToken.empty()) {
            Logger::logError("BetterReplays", "Failed to select display.");
            return;
        }
        settings.displays.push_back(config);
        settings.save();
        setupRecorder(config);
    }).detach();
}