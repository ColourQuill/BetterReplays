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

// std
#include <thread>
#include <chrono>

BetterReplays::~BetterReplays() {
    if (saveHotkey) {
        delete saveHotkey;
    }
    if (exitHotkey) {
        delete exitHotkey;
    }
    if (buffer) {
        delete buffer;
    }
    if (muxer) {
        delete muxer;
    }
    if (encoder) {
        delete encoder;
    }
    if (capture) {
        delete capture;
    }
    saveHotkey = nullptr;
    exitHotkey = nullptr;
    buffer = nullptr;
    muxer = nullptr;
    encoder = nullptr;
    capture = nullptr;
}

bool BetterReplays::start() {
    if (!settings.load()) {
        Logger::logError("BetterReplays", "Failed to load settings.");
        return false;
    }

    Logger::logInfo("BetterReplays", "Resolution: " + 
    std::to_string(settings.display.resolutionX) + "x" + 
    std::to_string(settings.display.resolutionY) + 
    " @ " + std::to_string(settings.display.fps));

    capture = ScreenCapture::create();
    encoder = Encoder::create();
    muxer = Muxer::create();
    buffer = new Buffer(Time::seconds(settings.replay.length));
    saveHotkey = Hotkey::create();
    exitHotkey = Hotkey::create();

    capture->setSettings(&settings);
    encoder->setSettings(&settings);
    muxer->setSettings(&settings);

    encoder->setPacketCallback([&](const EncodedPacket& packet) {
        buffer->addPacket(packet);
    });

    capture->setFrameCallback([&](const Frame& frame) {
        encoder->encode(frame);
    });

    capture->setEncoderInitCallback([&](uint32_t srcWidth, uint32_t srcHeight) {
        if (!encoder->init(srcWidth, srcHeight, settings.display.resolutionX, settings.display.resolutionY, settings.display.fps)) {
            Logger::logError("BetterReplays", "Failed to initialize encoder.");
        }
    });

    if (!capture->init()) {
        Logger::logError("BetterReplays", "Failed to initialize capture.");
        return false;
    }

    return true;
}
bool BetterReplays::run() {
    if (!capture->start()) {
        Logger::logError("BetterReplays", "Failed to start capture.");
        return false;
    }

    saveHotkey->start(Key::KEY_left_alt, Key::KEY_period, [&]() { save(); });
    exitHotkey->start(Key::KEY_left_alt, Key::KEY_comma, [&]() { stop(); });

    Logger::logInfo("BetterReplays", "Running - press (Alt + .) to save a clip && (Alt + ,) to exit.");

    while (capture->isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    saveHotkey->stop();
    exitHotkey->stop();

    return true;
}

bool BetterReplays::save() {
    std::string filename = "Replay " + Time::now() + "." + settings.replay.format;

    Logger::logInfo("BetterReplays", "Saving replay...");
    std::vector<EncodedPacket> snapshot = buffer->getSnapshot();

    if (!muxer->save(filename, snapshot, settings.display.resolutionX, settings.display.resolutionY, settings.display.fps)) {
        Logger::logError("BetterReplays", "Failed to save replay.");
        return false;
    }
    Logger::logInfo("BetterReplays", "Replay saved successfully to -> " + filename);
    
    return true;
}

void BetterReplays::stop() {
    capture->stop();
    Logger::logInfo("BetterReplays", "Shutdown complete.");
}