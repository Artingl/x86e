#pragma once

#include "cpu/im/x86im.h"
#include "utils/utils.h"

namespace x86e::im {

    // all instruction from 8086 till i386 intel CPUs

    class i386_InstructionsManager : public InstructionsManager {
    public:
        i386_InstructionsManager(cpu::CPU* cpu);
        ~i386_InstructionsManager();

        ADD_INSTRUCTION(add_rm8_r8);
        ADD_INSTRUCTION(add_rm16_32_r16_32);
        ADD_INSTRUCTION(add_r16_32_rm16_32);
        ADD_INSTRUCTION(add_r8_rm8);
        ADD_INSTRUCTION(add_al_imm8);
        ADD_INSTRUCTION(add_eAX_imm16_32);
        ADD_INSTRUCTION(halt);
        ADD_INSTRUCTION(push_es);
        ADD_INSTRUCTION(pop_es);
        ADD_INSTRUCTION(push_cs);
        ADD_INSTRUCTION(or_rm8_r8);
        ADD_INSTRUCTION(or_rm16_32_r16_32);
        ADD_INSTRUCTION(or_r8_rm8);
        ADD_INSTRUCTION(or_r16_32_rm16_32);
        ADD_INSTRUCTION(or_al_imm8);
        ADD_INSTRUCTION(or_eAX_imm16_32);
        ADD_INSTRUCTION(adc_rm8_r8);
        ADD_INSTRUCTION(adc_rm16_32_r16_32);
        ADD_INSTRUCTION(adc_r8_rm8);
        ADD_INSTRUCTION(adc_r16_32_rm16_32);
        ADD_INSTRUCTION(adc_al_imm8);
        ADD_INSTRUCTION(adc_eAX_imm16_32);
        ADD_INSTRUCTION(push_ss);
        ADD_INSTRUCTION(pop_ss);

    };

}
