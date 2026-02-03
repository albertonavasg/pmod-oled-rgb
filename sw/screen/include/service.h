#ifndef SERVICE_H
#define SERVICE_H

#include <vector>        // vector
#include <unordered_map> // unordered_map

#include <nlohmann/json.hpp>

#include "screen.h"

using json = nlohmann::json;

class Service {

    public:
        // Constructor
        explicit Service(const std::string &configFile);

        // Apply config from JSON
        void applyConfig(const std::string &configFile);
        // Get screen by Id
        Screen& screen(const std::string &id);

    private:
        std::vector<Screen> m_screens;
        std::unordered_map<std::string, size_t> m_screenIndex;

        json loadJson(const std::string &path) const;
        static screen::Orientation parseOrientation(const std::string &s);
};

#endif // SERVICE_H