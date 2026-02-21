#ifndef SERVICE_H
#define SERVICE_H

#include <vector>        // vector
#include <unordered_map> // unordered_map
#include <atomic>        // atomic
#include <string_view>   // string_view

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

        // Full test routine
        void runTests();

        // Main service loop
        void run();
        void stop();

    private:
        std::vector<service::ScreenContext> m_screens;

        std::atomic<bool> m_running{true};

        service::Date m_date{};
        service::Date m_prevDate{};
        bool m_dateHasChanged = false;
        service::Time m_time{};
        service::Time m_prevTime{};
        bool m_timeHasChanged = false;
        service::Network m_net{};
        service::Network m_prevNet{};
        bool m_netHasChanged = false;

        // Mode handlers
        void updateMode(service::ScreenContext &ctx);
        void updateNoneMode(service::ScreenContext &ctx);
        void updateInfoMode(service::ScreenContext &ctx);
        void updateDigitalClockMode(service::ScreenContext &ctx);
        void updateAnalogClockMode(service::ScreenContext &ctx);

        // Helpers
        json loadJson(const std::string &path) const;

        // Parse config file
        service::ScreenMode parseScreenMode(const std::string &s);
        service::ScreenSubMode parseScreenSubMode(const std::string &s);
        static screen::Orientation parseOrientation(const std::string &s);

        // Render
        bool renderTextBlock(Screen &s, const service::TextBlock &block, std::string_view text);

        void renderDateString(service::ScreenContext &ctx);
        void renderTimeString(service::ScreenContext &ctx, const bool forceFullRender);
        void renderIpString(service::ScreenContext &ctx);

        void renderAnalogClockFace(service::ScreenContext &ctx);
        void renderAnalogClockHands(service::ScreenContext &ctx, const bool forceFullRender);

        // Date, Time and IP updaters
        void updateDateAndTime();
        void updateIpAndMask();
        bool hasCarrier(const char *iface);
        std::string formatIPv4(uint32_t ip);
};

#endif // SERVICE_H