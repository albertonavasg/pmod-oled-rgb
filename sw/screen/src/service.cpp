#include <iostream> // cout, endl

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "service.h"

Service::Service(const std::vector<std::reference_wrapper<Screen>> &screens) {

    m_screens = screens;
}