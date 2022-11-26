#pragma once

#include "memory/memory.h"
#include "io/Logger.h"

#include <cstdint>
#include <vector>

#define OP_CHECK_PREFIX(VEC, PRE) std::count(VEC.begin(), VEC.end(), PRE)

namespace x86e::cpu {
    typedef uint32_t RegisterValue;
    typedef bool _1bit;

    enum Registers {
        EAX,
        ECX,
        EDX,
        EBX,
        ESP,
        EBP,
        ESI,
        EDI,
        EIP,

        AX,
        CX,
        DX,
        BX,

        AL,
        CL,
        DL,
        BL,
        AH,
        CH,
        DH,
        BH,

        SP,
        BP,
        SI,
        DI,
        IP,

        CS,
        DS,
        ES,
        FS,
        GS,
        SS,
    };

    enum Flags {
        // FLAGS
        CF = 0,
        PF = 2,
        AF = 4,
        ZF = 6,
        SF = 7,
        TF = 8,
        IF = 9,
        DF = 10,
        OF = 11,
        IOPL = 12, // 12 - 13
        NT = 14,

        // EFLAGS
        RF = 16,
        VM = 17,
    };

    struct Opcode {
        std::vector<uint8_t> prefixes;

        uint32_t beginIP;

        uint8_t instruction;

        uint8_t mod_or_index;
        uint8_t rm_or_ss;
        uint8_t modrm_or_sib_value;

    };

    enum InstructionPrefix {
        CS_OVERRIDE = 0x2E,
        SS_OVERRIDE = 0x36,
        DS_OVERRIDE = 0x3E,
        ES_OVERRIDE = 0x26,
        FS_OVERRIDE = 0x64,
        GS_OVERRIDE = 0x65,

        OPERAND_SIZE = 0x66,
        ADDRESS_SIZE = 0x67
    };

    class CPU {
    public:
        CPU(uint64_t memory);
        ~CPU();

        virtual void          reset();
        virtual void          cycle();

        void setRegister(Registers reg, RegisterValue value);
        RegisterValue getRegister(Registers reg);

        void setFlag(Flags flag, _1bit value);
        _1bit getFlag(Flags flag);

        void parseModRM(Opcode& opcode, uint64_t address);

        RegisterValue incGetRegister(Registers reg, uint32_t value);
        RegisterValue incGetRegister(Registers reg);
        RegisterValue decGetRegister(Registers reg);
        RegisterValue decGetRegister(Registers reg, uint32_t value);
        x86e::memory::Memory& getMemory();

        // side = true ; get from top side
        // side = false ; get from left side
        // http://ref.x86asm.net/coder32.html#modrm_byte_16
        uint32_t ModRMValue16bit(Opcode& opcode, bool side, uint8_t offset=0);
        uint32_t ModRMValue32bit(Opcode& opcode, bool side, uint8_t offset=0);
        uint32_t sibByte32bit(Opcode& opcode, bool side);

        void halt();
        bool isHalted();

        void pushOntoStackImm8(uint8_t value);
        void pushOntoStackImm16(uint16_t value);
        void pushOntoStackImm32(uint32_t value);

        uint8_t popFromStackImm8();
        uint16_t popFromStackImm16();
        uint32_t popFromStackImm32();

        bool longMode();

    private:

        x86e::memory::Memory _memory;
        uint32_t _registers[64];
        _1bit _flags[64];

    protected:
        bool _isHalted;

    };

}
