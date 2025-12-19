#ifndef SCREEN_TESTER_H
#define SCREEN_TESTER_H

#include <vector>

#include "screen.h"

class ScreenTester {

    public:
        explicit ScreenTester(const std::vector<std::reference_wrapper<Screen>>& screens);
        void testDisplay();
        void testRemapColorDepth();

        private:
            std::vector<std::reference_wrapper<Screen>> m_screens;
};

#endif // SCREEN_TESTER_H