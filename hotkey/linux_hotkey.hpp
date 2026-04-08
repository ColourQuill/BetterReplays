#ifndef __LINUX_HOTKEY_HPP__
#define __LINUX_HOTKEY_HPP__

#ifdef __linux__

#include <hotkey.hpp>

class LinuxHotkey : public Hotkey {
    public:
        bool start(int key1, int key2, HotkeyCallback callback) override;
        void stop() override;
    private:
        void listenLoop(int key1, int key2) override;
};

#endif

#endif