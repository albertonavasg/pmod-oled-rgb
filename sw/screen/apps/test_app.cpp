#include <iostream> // cout

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "screen_tester.h"

int main() {

    std::cout << "Screen test application running." << std::endl;

    std::unique_ptr<Screen> screenA;
    std::unique_ptr<Screen> screenB;

    try {
        screenA = std::make_unique<Screen>("uio0");
        screenB = std::make_unique<Screen>("uio1");
    } catch (const std::exception &e) {
        std::cerr << "Error initializing screens: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::reference_wrapper<Screen>> screens = {*screenA, *screenB};
    ScreenTester tester(screens);

    tester.testDisplay();
    tester.testRemapColorDepth();

    return 0;
}