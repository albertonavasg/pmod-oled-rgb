#include <iostream> // cout
#include <memory>   // unique_ptr
#include <chrono>   // time
#include <thread>   // sleep_for
#include <csignal>  // signal

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "service.h"

using namespace std::chrono_literals;

static Service* g_service = nullptr;

static void signalHandler(int);

int main() {

    std::cout << "Screen service application running." << std::endl;

    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    Service service("/home/petalinux/config.json");
    g_service = &service;

    service.run();

    return 0;
}

static void signalHandler(int) {

    if (g_service) {
        g_service->stop();
    }
}