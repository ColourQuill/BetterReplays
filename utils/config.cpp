#include <config.hpp>

// std
#include <fstream>
#include <cstring>

// better replays
#include <logger.hpp>

Config::Config(const std::string& path) {
    read(path);
}

std::string Config::get(std::string header, std::string key) {
    if (data.find(header) != data.end()) {
        if (data[header].data.find(key) != data[header].data.end()) {
            return data[header].data[key];
        }
    }
    return "";
}
int Config::getInt(std::string header, std::string key) {
    if (data.find(header) != data.end()) {
        if (data[header].data.find(key) != data[header].data.end()) {
            return std::stoi(data[header].data[key]);
        }
    }
    return 0;
}
float Config::getFloat(std::string header, std::string key) {
    if (data.find(header) != data.end()) {
        if (data[header].data.find(key) != data[header].data.end()) {
            return std::stof(data[header].data[key]);
        }
    }
    return 0.0f;
}
double Config::getDouble(std::string header, std::string key) {
    if (data.find(header) != data.end()) {
        if (data[header].data.find(key) != data[header].data.end()) {
            return std::stod(data[header].data[key]);
        }
    }
    return 0.0;
}
bool Config::getBool(std::string header, std::string key) {
    std::string value = data[header].data[key];
    if (value == "true"    ||
        value == "True"    ||
        value == "yes"     ||
        value == "Yes"     ||
        value == "on"      ||
        value == "On"      || 
        value == "enable"  ||
        value == "Enable"  ||
        value == "enabled" ||
        value == "Enabled" ||
        value == "1"
    ) {
        return true;
    } else {
        return false;
    }
}

void Config::set(std::string header, std::string key, std::string value) {
    data[header].data[key] = value;
}
void Config::set(std::string header, std::string key, int value) {
    data[header].data[key] = std::to_string(value);
}
void Config::set(std::string header, std::string key, float value) {
    data[header].data[key] = std::to_string(value);
}
void Config::set(std::string header, std::string key, double value) {
    data[header].data[key] = std::to_string(value);
}
void Config::set(std::string header, std::string key, bool value) {
    if (value) {
        data[header].data[key] = "true";
    } else {
        data[header].data[key] = "false";
    }
}

bool Config::read(const std::string& path) {
    this->path = path;
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::logError("Config", "Failed to open file -> " + path);
        return false;
    }

    std::vector<std::string> buffer;

    buffer.clear();
    std::string line;
    while (std::getline(file, line)) {
        buffer.push_back(line);
    }
    file.close();

    parse(buffer);
    buffer.clear();
    return true;
}
bool Config::save(const std::string& path) {
    this->path = path;
    return save();
}
bool Config::overwrite(const std::string& path) {
    this->path = path;
    return overwrite();
}
bool Config::save() {
    return overwrite();
}
bool Config::overwrite() {
    std::vector<std::string> buffer = {};
    for (auto& sectionKV : data) {
        std::string section = "[" + sectionKV.first + "]";
        write(buffer, section);

        int i = 0;
        int size = sectionKV.second.data.size();
        for (auto& kv : sectionKV.second.data) {
            std::string keyValue = kv.first + " = " + kv.second;
            write(buffer, keyValue);
            ++i;
            if (i >= size) {
                write(buffer, "");
            }
        }
    }
    return saveFile(buffer);
}

bool Config::parse(const std::vector<std::string>& buffer) {
    std::string currentSection;
    for (unsigned long long i = 0; i < buffer.size(); ++i) {
        std::string line = trim(buffer[i]);

        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            data[currentSection] = {};
        } else {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                data[currentSection].data[key] = value;
            } else {
                Logger::logWarning("Config", "Invalid line -> " + line);
                return false;
            }
        }
    }
    return true;
}

std::string Config::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

void Config::write(std::vector<std::string>& buffer, const std::string& line) {
    buffer.push_back(line);
}

bool Config::saveFile(std::vector<std::string>& buffer) {
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        Logger::logError("Config", "Failed to open file -> " + path);
        return false;
    }
    
    for (auto line : buffer) {
        file << line << '\n';
    }
    file.close();
    buffer.clear();

    return true;
}