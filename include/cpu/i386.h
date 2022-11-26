#pragma once

#include <cstdint>
#include "cpu.h"
#include "cpu/im/i386im.h"

namespace x86e::cpu {
    class i386 : public CPU {
    public:
        i386(uint32_t memory);
        ~i386();

        void cycle();

    private:
        im::i386_InstructionsManager _instructionsManager;

    };

}
