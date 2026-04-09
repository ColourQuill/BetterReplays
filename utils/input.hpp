#ifndef __INPUT_HPP__
#define __INPUT_HPP__

#ifdef _WIN32

#elif __linux__
    #include <linux/input-event-codes.h>

    enum Key {
        KEY_left_alt = KEY_LEFTALT,
        KEY_right_alt = KEY_RIGHTALT,
        KEY_left_shift = KEY_LEFTSHIFT,
        KEY_right_shift = KEY_RIGHTSHIFT,
        KEY_left_ctrl = KEY_LEFTCTRL,
        KEY_right_ctrl = KEY_RIGHTCTRL,
        KEY_comma = KEY_COMMA,
        KEY_period = KEY_DOT,
        KEY_slash = KEY_SLASH,
        KEY_left_bracket = KEY_LEFTBRACE,
        KEY_right_bracket = KEY_RIGHTBRACE
    };
#elif __APPLE__

#endif

#endif