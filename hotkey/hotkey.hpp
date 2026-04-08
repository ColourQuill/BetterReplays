#ifndef __HOTKEY_HPP__
#define __HOTKEY_HPP__

// std
#include <functional>
#include <thread>
#include <atomic>
#include <string>

using HotkeyCallback = std::function<void()>;

class Hotkey {
    public:
        ~Hotkey() = default;

        virtual bool start(int key1, int key2, HotkeyCallback callback) = 0;
        virtual void stop() = 0;

        static Hotkey* create();
    protected:
        virtual void listenLoop(int key1, int key2) = 0;

        std::thread listenThread;
        std::atomic<bool> running = false;
        HotkeyCallback hotkeyCallback;
};

#endif