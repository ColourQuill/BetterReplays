#include <hotkey.hpp>

#ifdef _WIN32
    #include <windows_hotkey.hpp>
#elif __APPLE__
    #include <mac_hotkey.hpp>
#elif __linux__
    #include <linux_hotkey.hpp>
#endif

Hotkey* Hotkey::create() {
#ifdef _WIN32
    return new WindowsHotkey();
#elif __APPLE__
    return new MacHotkey();
#elif __linux__
    return new LinuxHotkey();
#else
    static_assert(false, "Capture creation failed: Unsupported platform.");
#endif
}