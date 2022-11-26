#pragma once

#include "cpu/cpu.h"

#define REF_INSTRUCTION(CLASS, NAME) void CLASS::NAME(cpu::Opcode& opcode)
#define ADD_INSTRUCTION(NAME) virtual void NAME(cpu::Opcode& opcode)

// x86 instruction manager

namespace x86e::im {
    class InstructionsManager {
    public:
        InstructionsManager(cpu::CPU* cpu);
        ~InstructionsManager();

    protected:
        cpu::CPU* _cpu;

    };

}
