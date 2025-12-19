#ifndef TEST_H
#define TEST_H

#include <vector>      // vector
#include <utility>     // forward
#include <functional>  // reference_wrapper, invoke
#include <chrono>      // time, seconds, milliseconds
#include <thread>      // sleep_for

#include "screen.h"

class Test {

    public:
        // Constructor
        explicit Test(const std::vector<std::reference_wrapper<Screen>>& screens);

        // Test routines
        void display();
        void remapColorDepth();

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

#endif // TEST_H