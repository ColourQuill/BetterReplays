#include <timer.hpp>

void Timer::start() {
    m_start = std::chrono::high_resolution_clock::now();
}
double Timer::end() {
    m_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = m_end - m_start;
    return duration.count();
}