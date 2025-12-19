#ifndef SCREEN_TESTER_H
#define SCREEN_TESTER_H

#include <vector>  // vector
#include <utility> // forward

#include "screen.h"

class ScreenTester {

    public:
        // Constructor
        explicit ScreenTester(const std::vector<std::reference_wrapper<Screen>>& screens);

        // Test routines
        void testDisplay();
        void testRemapColorDepth();

        private:
            std::vector<std::reference_wrapper<Screen>> m_screens;
            // Broadcast command to all screens
            template <typename Rep, typename Period, typename... Args>
            void broadcastCommand(const std::chrono::duration<Rep, Period>& delay = std::chrono::duration<Rep, Period>(0), Args&&... args) {
                for (Screen& s : m_screens) {
                    s.sendCommand(std::forward<Args>(args)...);
                }
                if (delay.count() > 0) {
                    std::this_thread::sleep_for(delay);
                }
            }
};

#endif // SCREEN_TESTER_H