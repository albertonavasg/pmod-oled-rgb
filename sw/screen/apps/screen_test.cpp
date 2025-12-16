#include <iostream> // cout
#include <chrono>   // seconds, milliseconds
#include <thread>   // sleep_for

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

int main() {

    std::cout << "Screen test application running." << std::endl;

    try {
        Screen screenA("uio0");
        Screen screenB("uio1");

        screenA.setPowerState(true);
        screenB.setPowerState(true);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        screenA.sendCommand(screen::Command::EntireDisplayOn);
        screenB.sendCommand(screen::Command::EntireDisplayOn);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        screenA.sendCommand(screen::Command::NormalDisplay);
        screenB.sendCommand(screen::Command::NormalDisplay);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        screenA.setPowerState(false);
        screenB.setPowerState(false);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}