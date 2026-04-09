#ifndef __DISPLAY_CONFIG_HPP__
#define __DISPLAY_CONFIG_HPP__

// std
#include <string>

struct DisplayConfig {
    std::string name;
    std::string restoreToken;
    int saveHotkeyA;
    int saveHotkeyB;
};

#endif