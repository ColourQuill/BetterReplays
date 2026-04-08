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

    bool key1Held = false;
    bool key2Held = false;

    struct input_event ev;
    while (running) {
        for (int fd : fds) {
            while (read(fd, &ev, sizeof(ev)) > 0) {
                if (ev.type != EV_KEY) {
                    continue;
                }

                bool pressed = (ev.value == 1);  // 1=down, 0=up, 2=repeat

                if (ev.code == key1) {
                    key1Held = pressed;
                }
                if (ev.code == key2) {
                    key2Held = pressed;
                }

                if (key1Held && key2Held && pressed) {
                    Logger::logInfo("Hotkey", "Detected hotkey activation.");
                    if (hotkeyCallback) {
                        hotkeyCallback();
                    }
                    key1Held = key2Held = false;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int fd : fds) {
        close(fd);
    }
}