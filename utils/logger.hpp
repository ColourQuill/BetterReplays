#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

// std
#include <string>

class Logger {
    public:
        static void logInfo(const std::string& str);
        static void logError(const std::string& str);
        static void logWarning(const std::string& str);

        static void logInfo(const std::string& source, const std::string& contents);
        static void logError(const std::string& source, const std::string& contents);
        static void logWarning(const std::string& source, const std::string& contents);

        static std::string ptrToStr(const void* p);
    private:
        static std::string getTime();
};

#endif