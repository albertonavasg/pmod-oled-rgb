#ifndef SCREEN_TESTER_H
#define SCREEN_TESTER_H

#include <vector>      // vector
#include <utility>     // forward
#include <functional>  // reference_wrapper, invoke
#include <type_traits> // is_invocable_v

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

            // Broadcast method to all screens
            template <typename F, typename... Args>
            void broadcast(F&& f, std::chrono::milliseconds delay = std::chrono::milliseconds(0), Args&&... args)
            {
                for (Screen& s : m_screens) {
                    std::invoke(std::forward<F>(f), s, std::forward<Args>(args)...);
                }
                if (delay.count() > 0) {
                    std::this_thread::sleep_for(delay);
                }
            }
};

#endif // SCREEN_TESTER_H