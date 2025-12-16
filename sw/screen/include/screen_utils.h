#ifndef SCREEN_UTILS_H
#define SCREEN_UTILS_H

#include <memory>
#include <string>

#include "screen.h"

std::runtime_error sys_error(const std::string &msg) ;

std::unique_ptr<Screen> createScreen(const std::string &uio);

#endif // SCREEN_UTILS_H
