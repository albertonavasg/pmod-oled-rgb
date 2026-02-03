#include <iostream> // cout
#include <memory>   // unique_ptr
#include <chrono>   // time
#include <thread>   // sleep_for

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "service.h"

using namespace std::chrono_literals;

int main() {

    std::cout << "Screen service application running." << std::endl;

    Service service("/home/petalinux/config.json");
    std::this_thread::sleep_for(500ms);

    service.screen("A").clearScreen();
    service.screen("B").clearScreen();
    std::this_thread::sleep_for(500ms);

    service.screen("A").drawString("Screen A", screen::StandardColor::White);
    std::this_thread::sleep_for(500ms);
    service.screen("B").drawString("Screen B", screen::StandardColor::White);
    std::this_thread::sleep_for(500ms);

    service.screen("A").clearScreen();
    service.screen("B").clearScreen();
    std::this_thread::sleep_for(500ms);

    service.runTests();
    service.applyConfig("/home/petalinux/config.json");

    return 0;
}