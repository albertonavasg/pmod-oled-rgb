#ifndef SERVICE_H
#define SERVICE_H

#include <vector>        // vector
#include <unordered_map> // unordered_map

#include <nlohmann/json.hpp>

#include "service_constants.h"
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

        void runTests();

        void run();

    private:
        std::vector<Screen> m_screens;
        std::vector<service::ScreenMode> m_modes;
        std::unordered_map<std::string, size_t> m_screenIndex;

        std::string m_date = "";
        std::string m_time = "";
        std::string m_ip   = "";
        std::string m_mask = "";

        void enterMode(size_t index);
        void enterNoneMode(Screen &s);
        void enterInfoMode(Screen &s);
        void enterDigitalClockMode(Screen &s);
        void enterAnalogClockMode(Screen &s);

        void updateMode(size_t index);
        void updateNoneMode(Screen &s);
        void updateInfoMode(Screen &s);
        void updateDigitalClockMode(Screen &s);
        void updateAnalogClockMode(Screen &s);

        // Helpers
        json loadJson(const std::string &path) const;

        service::ScreenMode parseScreenMode(const std::string &s);
        static screen::Orientation parseOrientation(const std::string &s);

        bool updateDateAndTime();
        bool updateIpAndMask();
        bool hasCarrier(const char *iface);

};

#endif // SERVICE_H