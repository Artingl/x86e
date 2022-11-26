#include "io/Logger.h"

#include <cstdarg>
#include <chrono>

#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>


namespace x86e::io {
    uint64_t startTime = -1;

    void debug_print(Level type, const std::string message, ...) {
        // todo: move any logging to separated thread
        if (startTime == -1)
            startTime = timestamp();

        struct winsize w;
        ioctl(fileno(stdout), TIOCGWINSZ, &w);
        int32_t size = w.ws_col <= 0 ? 80 : w.ws_col;

        const char * const zcFormat = message.c_str();
        char* logType = (char *)(type == Level::NOTICE ? "NOTICE" : type == Level::INFO ? "INFO" :
                                 type == Level::WARNING ? "WARNING" : type == Level::ERROR ? "ERROR" : "CRITICAL");

        printf(" [%f] %s : ", (timestamp() - startTime) / 1000.f, logType);

        va_list args;
        va_start(args, message);
        int32_t iLen = std::vprintf(zcFormat, args);
        va_end(args);

//        for (size_t i = 0; i < size - iLen - std::strlen(logType) - 16; i++)
//            putc(' ', stdout);

        printf("\n");
    }

    uint64_t timestamp() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
}
