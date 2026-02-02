#ifndef SERVICE_H
#define SERVICE_H

#include <vector>      // vector
#include <functional>  // reference_wrapper

#include "screen.h"

class Service {

    public:
        // Constructor
        explicit Service(const std::vector<std::reference_wrapper<Screen>> &screens);

    private:
        std::vector<std::reference_wrapper<Screen>> m_screens;
};

#endif // SERVICE_H