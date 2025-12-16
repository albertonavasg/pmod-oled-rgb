#ifndef SCREEN_TESTER_H
#define SCREEN_TESTER_H

#include <vector>

#include "screen.h"

class ScreenTester {

    public:
        explicit ScreenTester(std::vector<Screen*>& screens);
        void testDisplay();

        private:
            std::vector<Screen*> m_screens;
};

#endif // SCREEN_TESTER_H