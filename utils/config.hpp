#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

// std
#include <vector>
#include <string>
#include <map>

struct Config {
    public:
        Config(const std::string& path);
        Config() = default;
        ~Config() = default;

        std::string get(std::string header, std::string key);
        int getInt(std::string header, std::string key);
        float getFloat(std::string header, std::string key);
        double getDouble(std::string header, std::string key);
        bool getBool(std::string header, std::string key);

        void set(std::string header, std::string key, std::string value);
        void set(std::string header, std::string key, int value);
        void set(std::string header, std::string key, float value);
        void set(std::string header, std::string key, double value);
        void set(std::string header, std::string key, bool value);

        bool read(const std::string& path);
        bool save(const std::string& path);
        bool overwrite(const std::string& path);
        bool save();
        bool overwrite();
    private:
        struct Section {
            std::map<std::string, std::string> data;
        };

        std::string path;
        std::map<std::string, Section> data;

        bool parse(const std::vector<std::string>& buffer);
        std::string trim(const std::string& str);
        void write(std::vector<std::string>& buffer, const std::string& line);
        bool saveFile(std::vector<std::string>& buffer);
};

#endif