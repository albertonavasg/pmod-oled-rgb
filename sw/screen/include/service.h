#ifndef SERVICE_H
#define SERVICE_H

#include <vector>        // vector
#include <unordered_map> // unordered_map
#include <atomic>        // atomic

#include <nlohmann/json.hpp>

#include "service_constants.h"
#include "screen.h"

using json = nlohmann::json;

class Service {

    public:
        // Constructor and destructor
        explicit Service(const std::string &configFile);
        ~Service();

        // Apply config from JSON
        void applyConfig(const std::string &configFile);
        // Get screen by Id
        Screen& screen(const std::string &id);

        // Full test routine
        void runTests();

        // Main service loop
        void run();
        void stop();

    private:
        std::vector<Screen> m_screens;
        std::vector<service::ScreenMode> m_screenModes;
        std::vector<service::ScreenSubMode> m_screenSubModes;
        std::unordered_map<std::string, size_t> m_screenIndex;

        std::atomic<bool> m_running{true};

        service::TimeData m_time{};
        service::TimeData m_prevTime{};
        bool m_timeHasChanged = false;
        service::NetworkData m_net{};
        service::NetworkData m_prevNet{};
        bool m_netHasChanged = false;

        // Mode handlers
        void enterMode(size_t index);
        void enterNoneMode(Screen &s, size_t index);
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

        // Parse config file
        service::ScreenMode parseScreenMode(const std::string &s);
        service::ScreenSubMode parseScreenSubMode(const std::string &s);
        static screen::Orientation parseOrientation(const std::string &s);

        // Render
        bool renderTextBlock(Screen &s, const service::TextBlock &block, const std::string text);

        void renderDateString(Screen &s);
        void renderTimeString(Screen &s);
        void renderIpString(Screen &s);

        // Date Time and IP updaters
        bool updateDateAndTime();
        bool updateIpAndMask();
        bool hasCarrier(const char *iface);
        std::string formatIPv4(uint32_t ip);

};

#endif // SERVICE_H