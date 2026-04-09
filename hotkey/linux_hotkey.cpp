#include <linux_hotkey.hpp>

// better replays
#include <logger.hpp>

// linux
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

// std
#include <string>
#include <vector>
#include <iostream>

static std::vector<std::string> findKeyboards() {
    std::vector<std::string> devices;
    DIR* dir = opendir("/dev/input");
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.find("event") != std::string::npos) {
            devices.push_back("/dev/input/" + name);
        }
    }
    closedir(dir);
    return devices;
}

bool isKeyDown(int fd, int keyCode) {
    uint8_t keyBits[KEY_MAX / 8 + 1] = {};
    if (ioctl(fd, EVIOCGKEY(sizeof(keyBits)), keyBits) < 0) return false;
    return keyBits[keyCode / 8] & (1 << (keyCode % 8));
}
bool isKeyPressed(const input_event& ev, int keyCode) {
    return ev.type == EV_KEY && ev.code == keyCode && ev.value == 1;
}

bool LinuxHotkey::start(int key1, int key2, HotkeyCallback callback) {
    hotkeyCallback = std::move(callback);
    running  = true;
    listenThread = std::thread(&LinuxHotkey::listenLoop, this, key1, key2);
    return true;
}
void LinuxHotkey::stop() {
    running = false;
    if (listenThread.joinable()) {
        listenThread.join();
    }
}
void LinuxHotkey::listenLoop(int key1, int key2) {
    auto devices = findKeyboards();
    std::vector<int> fds;
    for (auto& path : devices) {
        int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd >= 0) fds.push_back(fd);
    }

    struct input_event ev;
    while (running) {
        for (int fd : fds) {
            while (read(fd, &ev, sizeof(ev)) > 0) {
                if (ev.type != EV_KEY) continue;

                if (isKeyPressed(ev, key2) && isKeyDown(fd, key1)) {
                    Logger::logInfo("Hotkey", "Detected hotkey activation.");
                    if (hotkeyCallback) hotkeyCallback();
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int fd : fds) close(fd);
}