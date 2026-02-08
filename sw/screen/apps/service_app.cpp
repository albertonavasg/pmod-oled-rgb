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

    service.run();
    std::this_thread::sleep_for(1s);

    return 0;
}