#include "cpu/i386.h"


namespace x86e::cpu {

    i386::i386(uint32_t memory)
        : CPU::CPU(memory), _instructionsManager(this) {
    }

    i386::~i386() {
    }

    void i386::cycle() {
        if (_isHalted)
            return;

        cpu::Opcode opcode;
        memory::Memory& memory = getMemory();

        opcode.beginIP = getRegister(EIP);
        opcode.instruction = memory.readImm8(getRegister(EIP));

        bool isPrefix = true;

        while (isPrefix) {
            // check if this instruction has prefixes
            switch (opcode.instruction) {
                case InstructionPrefix::CS_OVERRIDE:
                case InstructionPrefix::SS_OVERRIDE:
                case InstructionPrefix::DS_OVERRIDE:
                case InstructionPrefix::ES_OVERRIDE:
                case InstructionPrefix::FS_OVERRIDE:
                case InstructionPrefix::GS_OVERRIDE:
                case InstructionPrefix::OPERAND_SIZE:
                case InstructionPrefix::ADDRESS_SIZE:
                    opcode.prefixes.push_back(opcode.instruction);
                    opcode.instruction = memory.readImm8(incGetRegister(EIP));
                    break;

                default:
                    isPrefix = false;
            }
        }

        // parse instruction
        switch (opcode.instruction) {
            case 0x00:  // 	add	r/m8 , r8
                _instructionsManager.add_rm8_r8(opcode);
                break;

            case 0x01:  // 	add	r/m16/32 , r16/32
                _instructionsManager.add_rm16_32_r16_32(opcode);
                break;

            case 0x02:  // 	add	r8 , r/m8
                _instructionsManager.add_r8_rm8(opcode);
                break;

            case 0x03:  // 	add	r16/32 , r/m16/32
                _instructionsManager.add_r16_32_rm16_32(opcode);
                break;

            case 0x04:  // 	add	al , imm8
                _instructionsManager.add_al_imm8(opcode);
                break;

            case 0x05:  // 	add	eAX , imm16/32
                _instructionsManager.add_eAX_imm16_32(opcode);
                break;

            case 0x06:  // 	push es
                _instructionsManager.push_es(opcode);
                break;

            case 0x07:  // 	pop es
                _instructionsManager.pop_es(opcode);
                break;

            case 0x08:  // 	or r/m8 , r8
                _instructionsManager.or_rm8_r8(opcode);
                break;

            case 0x9:  // 	or r/m16/32 , r16/32
                _instructionsManager.or_rm16_32_r16_32(opcode);
                break;

            case 0x0a:  // 	or r8 , r/m8
                _instructionsManager.or_r8_rm8(opcode);
                break;

            case 0x0b:  // 	or r16/32 , r/m16/32
                _instructionsManager.or_r16_32_rm16_32(opcode);
                break;

            case 0x0c:  // 	or al , imm8
                _instructionsManager.or_al_imm8(opcode);
                break;

            case 0x0d:  // 	or eAX , imm16/32
                _instructionsManager.or_eAX_imm16_32(opcode);
                break;

            case 0x0e:  // 	push cs
                _instructionsManager.push_cs(opcode);
                break;

            case 0x0f:  //  two-byte instructions
                break;

            case 0x10:  // 	adc r/m8 , r8
                _instructionsManager.adc_rm8_r8(opcode);
                break;

            case 0x11:  // 	adc r/m16/32 , r16/32
                _instructionsManager.adc_rm16_32_r16_32(opcode);
                break;

            case 0x12:  // 	adc r8, r/m8
                _instructionsManager.adc_r8_rm8(opcode);
                break;

            case 0x13:  // 	adc r16/32 , r/m16/32
                _instructionsManager.adc_r16_32_rm16_32(opcode);
                break;

            case 0x14:  // 	adc al , imm8
                _instructionsManager.adc_al_imm8(opcode);
                break;

            case 0x15:  // 	adc eAX, imm16/32
                _instructionsManager.adc_eAX_imm16_32(opcode);
                break;

            case 0x16:  // 	push ss
                _instructionsManager.push_ss(opcode);
                break;

            case 0x17:  // 	pop ss
                _instructionsManager.pop_ss(opcode);
                break;

            case 0xf4:
                _instructionsManager.halt(opcode);
                break;

            default:
                io::debug_print(io::WARNING, "Invalid opcode 0x%02x!!! EIP=0x%x",
                                opcode.instruction,
                                getRegister(EIP));
                break;
        }

        incGetRegister(EIP);
    }

}
