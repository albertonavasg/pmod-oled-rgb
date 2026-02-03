#ifndef SERVICE_H
#define SERVICE_H

#include <vector>        // vector
#include <unordered_map> // unordered_map

#include "screen.h"

class Service {

    public:
        // Constructor
        explicit Service(const std::string &configFile);

        // Get screen by Id
        Screen& screen(const std::string& id);

    private:
        std::vector<Screen> m_screens;
        std::unordered_map<std::string, size_t> m_screenIndex;

        static screen::Orientation parseOrientation(const std::string &s);
};

#endif // SERVICE_H