#include <iostream>  // cerr
#include <stdexcept> // runtime_error
#include <cstring>   // strerror
#include <cerrno>    // errno

#include "screen.h"
#include "screen_utils.h"

std::runtime_error sys_error(const std::string &msg) {
    return std::runtime_error(msg + ": " + strerror(errno));
}

std::unique_ptr<Screen> createScreen(const std::string &uio) {

    try {
        return std::make_unique<Screen>(uio);
    } catch (const std::exception &e) {
        std::cerr << "Failed to create screen " << uio << ": " << e.what() << std::endl;
        return nullptr;
    }
}