#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <chrono>

class Timer {
    public:
        void start();
        double end();
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_end;
};

#endif