#include <iostream> // cout
#include <chrono>   // seconds, milliseconds
#include <thread>   // sleep_for

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "screen_utils.h"
#include "screen_tester.h"

int main() {

    std::cout << "Screen test application running." << std::endl;

    auto screenA = createScreen("uio0");
    auto screenB = createScreen("uio1");

    std::vector<std::reference_wrapper<Screen>> screens = {*screenA, *screenB};
    ScreenTester tester(screens);

    tester.testDisplay();

    return 0;
}