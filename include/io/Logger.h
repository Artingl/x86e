#pragma once

#include <string>
#include <cstdio>
#include <cstdint>

namespace x86e::io {
    enum Level {
        NOTICE, INFO, WARNING, ERROR, CRITICAL
    };

    void debug_print(Level type, const std::string message, ...);

    uint64_t timestamp();

}
