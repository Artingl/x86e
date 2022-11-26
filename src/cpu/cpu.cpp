#include <cstdint>
#include "cpu/cpu.h"

namespace x86e::cpu {

    CPU::CPU(uint64_t memory)
        : _memory(memory) {

    }

    CPU::~CPU() {
    }

    void CPU::setRegister(Registers reg, RegisterValue value) {
        switch (reg) {
            case AX:
            case CX:
            case DX:
            case BX:
                this->_registers[reg - 9] = (this->_registers[reg - 9] & 0xffff0000) | (uint16_t)value;
                break;

            case AL:
            case CL:
            case DL:
            case BL:
                *((uint8_t*)&this->_registers[reg - 13]) = value;
                break;

            case AH:
            case CH:
            case DH:
            case BH:
                *((uint8_t*)&this->_registers[reg - 17] + 1) = value;
                break;

            case SP:
            case BP:
            case SI:
            case DI:
            case IP:
                this->_registers[reg - 17] = (this->_registers[reg - 17] & 0xffff0000) | (uint16_t)value;
                break;

            default:
                this->_registers[reg] = value;
        }
    }

    RegisterValue CPU::getRegister(Registers reg) {
        switch (reg) {
            case AX:
            case CX:
            case DX:
            case BX:
                return (uint16_t)(this->_registers[reg - 9] & 0x0000ffff);

            case AL:
            case CL:
            case DL:
            case BL:
                return (uint8_t)(this->_registers[reg - 13] & 0xffff00ff);

            case AH:
            case CH:
            case DH:
            case BH:
                return (uint8_t)((this->_registers[reg - 17] & 0xffffff00) >> 8);

            case SP:
            case BP:
            case SI:
            case DI:
            case IP:
                return (uint16_t)(this->_registers[reg - 17] & 0x0000ffff);

            default:
                return (uint32_t)this->_registers[reg];
        }
    }

    void CPU::setFlag(Flags flag, _1bit value) {
        _flags[flag] = value;
    }

    _1bit CPU::getFlag(Flags flag) {
        return _flags[flag];
    }

    RegisterValue CPU::incGetRegister(Registers reg) {
        RegisterValue currVal = getRegister(reg)+1;
        setRegister(reg, currVal);
        return currVal;
    }

    RegisterValue CPU::incGetRegister(Registers reg, uint32_t value) {
        RegisterValue currVal = getRegister(reg);
        setRegister(reg, currVal+value);
        return currVal+1;
    }

    RegisterValue CPU::decGetRegister(Registers reg) {
        RegisterValue currVal = getRegister(reg)-1;
        setRegister(reg, currVal);
        return currVal;
    }

    RegisterValue CPU::decGetRegister(Registers reg, uint32_t value) {
        RegisterValue currVal = getRegister(reg);
        setRegister(reg, currVal-value);
        return currVal-1;
    }

    void CPU::cycle() {
    }

    x86e::memory::Memory &CPU::getMemory() {
        return _memory;
    }

    void CPU::reset() {
        _isHalted = false;

        for (int i = 0; i < 64; i++)
            setRegister((Registers)(i), 0);

        for (int i = 0; i < 64; i++)
            setFlag((Flags)(i), 0);

        setRegister(EIP, 0);
        setRegister(ESP, 0xffff);

        setFlag((Flags) 1, 1);
        setFlag((Flags) 15, 0);
    }

    void CPU::parseModRM(Opcode &opcode, uint64_t address) {
        opcode.modrm_or_sib_value = _memory.readImm8(address);
        opcode.mod_or_index = opcode.modrm_or_sib_value >> 6;
        opcode.rm_or_ss = opcode.modrm_or_sib_value & 0b00000111;
    }

    uint32_t CPU::ModRMValue16bit(Opcode& opcode, bool side, uint8_t offset) {
#define GET_REGISTER16(FROM)                                                \
        switch (opcode.modrm_or_sib_value >> 4) {                           \
            case FROM:                                                      \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? AL : offset == 1 ? AX : EAX;       \
                return offset == 0 ? CL : offset == 1 ? CX : ECX;           \
                                                                            \
            case FROM + 1:                                                  \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? DL : offset == 1 ? DX : EDX;       \
                return offset == 0 ? BL : offset == 1 ? BX : EBX;           \
                                                                            \
            case FROM + 2:                                                  \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? AH : offset == 1 ? SP : ESP;       \
                return offset == 0 ? CH : offset == 1 ? BP : EBP;           \
                                                                            \
            case FROM + 3:                                                  \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? DH : offset == 1 ? SI : ESI;       \
                return offset == 0 ? BH : offset == 1 ? DI : EDI;           \
        }

        // todo: this is a mess. I'll make it better someday
        switch (opcode.mod_or_index) {
            case 0b00:
                if (side) {
                    GET_REGISTER16(0x0)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return getRegister(BX) + getRegister(SI);
                    if (opcode.rm_or_ss == 0x1) return getRegister(BX) + getRegister(DI);
                    if (opcode.rm_or_ss == 0x2) return getRegister(BP) + getRegister(SI);
                    if (opcode.rm_or_ss == 0x3) return getRegister(BP) + getRegister(DI);
                    if (opcode.rm_or_ss == 0x4) return getRegister(SI);
                    if (opcode.rm_or_ss == 0x5) return getRegister(DI);
                    if (opcode.rm_or_ss == 0x6) return _memory.readImm16(incGetRegister(EIP, 2));
                    if (opcode.rm_or_ss == 0x7) return getRegister(BX);
                }
                break;

            case 0b01:
                if (side) {
                    GET_REGISTER16(0x40)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return getRegister(BX) + getRegister(SI) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x1) return getRegister(BX) + getRegister(DI) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x2) return getRegister(BP) + getRegister(SI) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x3) return getRegister(BP) + getRegister(DI) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x4) return getRegister(SI) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x5) return getRegister(DI) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x6) return getRegister(BP) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x7) return getRegister(BX) + _memory.readImm8(incGetRegister(EIP));
                }
                break;

            case 0b10:
                if (side) {
                    GET_REGISTER16(0x80)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return getRegister(BX) + getRegister(SI) + _memory.readImm16(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x1) return getRegister(BX) + getRegister(DI) + _memory.readImm16(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x2) return getRegister(BP) + getRegister(SI) + _memory.readImm16(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x3) return getRegister(BP) + getRegister(DI) + _memory.readImm16(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x4) return getRegister(SI) + _memory.readImm16(incGetRegister(EIP, 2));
                    if (opcode.rm_or_ss == 0x5) return getRegister(DI) + _memory.readImm16(incGetRegister(EIP, 2));
                    if (opcode.rm_or_ss == 0x6) return getRegister(BP) + _memory.readImm16(incGetRegister(EIP, 2));
                    if (opcode.rm_or_ss == 0x7) return getRegister(BX) + _memory.readImm16(incGetRegister(EIP, 2));
                }
                break;

            case 0b11:
                if (side) {
                    GET_REGISTER16(0xC)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return offset == 0 ? AL : offset == 1 ? AX : EAX;
                    if (opcode.rm_or_ss == 0x1) return offset == 0 ? CL : offset == 1 ? CX : ECX;
                    if (opcode.rm_or_ss == 0x2) return offset == 0 ? DL : offset == 1 ? DX : EDX;
                    if (opcode.rm_or_ss == 0x3) return offset == 0 ? BL : offset == 1 ? BX : EBX;
                    if (opcode.rm_or_ss == 0x4) return offset == 0 ? AH : offset == 1 ? SP : ESP;
                    if (opcode.rm_or_ss == 0x5) return offset == 0 ? CH : offset == 1 ? BP : EBP;
                    if (opcode.rm_or_ss == 0x6) return offset == 0 ? DH : offset == 1 ? SI : ESI;
                    if (opcode.rm_or_ss == 0x7) return offset == 0 ? BH : offset == 1 ? DI : EDI;
                }

                break;

            default:
                // huh?
                return 0;
        }

        return 0;
    }

    uint32_t CPU::ModRMValue32bit(Opcode &opcode, bool side, uint8_t offset) {
#define GET_REGISTER32(FROM)                                                \
        switch (opcode.modrm_or_sib_value >> 4) {                           \
            case FROM:                                                      \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? AL : offset == 1 ? AX : EAX;       \
                return offset == 0 ? CL : offset == 1 ? CX : ECX;           \
                                                                            \
            case FROM + 1:                                                  \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? DL : offset == 1 ? DX : EDX;       \
                return offset == 0 ? BL : offset == 1 ? BX : EBX;           \
                                                                            \
            case FROM + 2:                                                  \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? AH : offset == 1 ? SP : ESP;       \
                return offset == 0 ? CH : offset == 1 ? BP : EBP;           \
                                                                            \
            case FROM + 3:                                                  \
                if ((opcode.modrm_or_sib_value & 0xf) <= 0b111)             \
                    return offset == 0 ? DH : offset == 1 ? SI : ESI;       \
                return offset == 0 ? BH : offset == 1 ? DI : EDI;           \
        }

        // todo: this is a mess. I'll make it better someday
        switch (opcode.mod_or_index) {
            case 0b00:
                if (side) {
                    GET_REGISTER32(0x0)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return getRegister(EAX);
                    if (opcode.rm_or_ss == 0x1) return getRegister(ECX);
                    if (opcode.rm_or_ss == 0x2) return getRegister(EDX);
                    if (opcode.rm_or_ss == 0x3) return getRegister(EBX);
                    if (opcode.rm_or_ss == 0x4) return sibByte32bit(opcode, side);
                    if (opcode.rm_or_ss == 0x5) return _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x6) return getRegister(ESI);
                    if (opcode.rm_or_ss == 0x7) return getRegister(EDI);
                }
                break;

            case 0b01:
                if (side) {
                    GET_REGISTER32(0x40)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return getRegister(EAX) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x1) return getRegister(ECX) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x2) return getRegister(EDX) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x3) return getRegister(EBX) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x4) return sibByte32bit(opcode, side) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x5) return getRegister(EBP) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x6) return getRegister(ESI) + _memory.readImm8(incGetRegister(EIP));
                    if (opcode.rm_or_ss == 0x7) return getRegister(EDI) + _memory.readImm8(incGetRegister(EIP));
                }
                break;

            case 0b10:
                if (side) {
                    GET_REGISTER32(0x80)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return getRegister(EAX) + _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x1) return getRegister(ECX) + _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x2) return getRegister(EDX) + _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x3) return getRegister(EBX) + _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x4) return sibByte32bit(opcode, side) + _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x5) return getRegister(EBP) + _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x6) return getRegister(ESI) + _memory.readImm32(incGetRegister(EIP, 4));
                    if (opcode.rm_or_ss == 0x7) return getRegister(EDI) + _memory.readImm32(incGetRegister(EIP, 4));
                }
                break;

            case 0b11:
                if (side) {
                    GET_REGISTER32(0xC)
                }
                else {
                    if (opcode.rm_or_ss == 0x0) return offset == 0 ? AL : offset == 1 ? AX : EAX;
                    if (opcode.rm_or_ss == 0x1) return offset == 0 ? CL : offset == 1 ? CX : ECX;
                    if (opcode.rm_or_ss == 0x2) return offset == 0 ? DL : offset == 1 ? DX : EDX;
                    if (opcode.rm_or_ss == 0x3) return offset == 0 ? BL : offset == 1 ? BX : EBX;
                    if (opcode.rm_or_ss == 0x4) return offset == 0 ? AH : offset == 1 ? SP : ESP;
                    if (opcode.rm_or_ss == 0x5) return offset == 0 ? CH : offset == 1 ? BP : EBP;
                    if (opcode.rm_or_ss == 0x6) return offset == 0 ? DH : offset == 1 ? SI : ESI;
                    if (opcode.rm_or_ss == 0x7) return offset == 0 ? BH : offset == 1 ? DI : EDI;
                }

                break;

            default:
                // huh?
                return 0;
        }

        return 0;
    }

    uint32_t CPU::sibByte32bit(Opcode &opcode, bool side) {
#define GET_REGISTER_SIB                                                    \
        switch (opcode.modrm_or_sib_value >> 4) {                           \
            case 0x00:                                                      \
            case 0x08:                                                      \
                return EAX;                                                 \
                                                                            \
            case 0x01:                                                      \
            case 0x09:                                                      \
                return ECX;                                                 \
                                                                            \
            case 0x02:                                                      \
            case 0x0A:                                                      \
                return EDX;                                                 \
                                                                            \
            case 0x03:                                                      \
            case 0x0B:                                                      \
                return EBX;                                                 \
                                                                            \
            case 0x04:                                                      \
            case 0x0C:                                                      \
                return ESP;                                                 \
                                                                            \
            case 0x05:                                                      \
            case 0x0D:                                                      \
                return opcode.mod_or_index == 0b00 ? _memory.readImm32(incGetRegister(EIP, 4)) \
                        : opcode.mod_or_index == 0b01 ? getRegister(EBP) + _memory.readImm8(incGetRegister(EIP)) : \
                                                        getRegister(EBP) + _memory.readImm32(incGetRegister(EIP, 4)); \
                                                                            \
            case 0x06:                                                      \
            case 0x0E:                                                      \
                return ESI;                                                 \
                                                                            \
            case 0x07:                                                      \
            case 0x0F:                                                      \
                return EDI;                                                 \
                                                                            \
        }

        if (side)
            GET_REGISTER_SIB

        switch (opcode.mod_or_index) {
            case 0b00:
                if (opcode.rm_or_ss == 0x0) return getRegister(EAX);
                if (opcode.rm_or_ss == 0x1) return getRegister(ECX);
                if (opcode.rm_or_ss == 0x2) return getRegister(EDX);
                if (opcode.rm_or_ss == 0x3) return getRegister(EBX);
                if (opcode.rm_or_ss == 0x4) return 0;
                if (opcode.rm_or_ss == 0x5) return getRegister(EBP);
                if (opcode.rm_or_ss == 0x6) return getRegister(ESI);
                if (opcode.rm_or_ss == 0x7) return getRegister(EDI);
                break;

            case 0b01:
                if (opcode.rm_or_ss == 0x0) return getRegister(EAX) * 2;
                if (opcode.rm_or_ss == 0x1) return getRegister(ECX) * 2;
                if (opcode.rm_or_ss == 0x2) return getRegister(EDX) * 2;
                if (opcode.rm_or_ss == 0x3) return getRegister(EBX) * 2;
                if (opcode.rm_or_ss == 0x4) return 0;
                if (opcode.rm_or_ss == 0x5) return getRegister(EBP) * 2;
                if (opcode.rm_or_ss == 0x6) return getRegister(ESI) * 2;
                if (opcode.rm_or_ss == 0x7) return getRegister(EDI) * 2;
                break;

            case 0b10:
                if (opcode.rm_or_ss == 0x0) return getRegister(EAX) * 4;
                if (opcode.rm_or_ss == 0x1) return getRegister(ECX) * 4;
                if (opcode.rm_or_ss == 0x2) return getRegister(EDX) * 4;
                if (opcode.rm_or_ss == 0x3) return getRegister(EBX) * 4;
                if (opcode.rm_or_ss == 0x4) return 0;
                if (opcode.rm_or_ss == 0x5) return getRegister(EBP) * 4;
                if (opcode.rm_or_ss == 0x6) return getRegister(ESI) * 4;
                if (opcode.rm_or_ss == 0x7) return getRegister(EDI) * 4;
                break;

            case 0b11:
                if (opcode.rm_or_ss == 0x0) return getRegister(EAX) * 8;
                if (opcode.rm_or_ss == 0x1) return getRegister(ECX) * 8;
                if (opcode.rm_or_ss == 0x2) return getRegister(EDX) * 8;
                if (opcode.rm_or_ss == 0x3) return getRegister(EBX) * 8;
                if (opcode.rm_or_ss == 0x4) return 0;
                if (opcode.rm_or_ss == 0x5) return getRegister(EBP) * 8;
                if (opcode.rm_or_ss == 0x6) return getRegister(ESI) * 8;
                if (opcode.rm_or_ss == 0x7) return getRegister(EDI) * 8;
                break;

            default:
                // huh?
                return 0;
        }

        return 0;
    }

    void CPU::pushOntoStackImm8(uint8_t value) {
        _memory.writeImm8(value, decGetRegister(ESP));
    }

    void CPU::pushOntoStackImm16(uint16_t value) {
        _memory.writeImm16(value, decGetRegister(ESP, 2));
    }

    void CPU::pushOntoStackImm32(uint32_t value) {
        _memory.writeImm32(value, decGetRegister(ESP, 4));
    }

    uint8_t CPU::popFromStackImm8() {
        return _memory.readImm8(incGetRegister(ESP));
    }

    uint16_t CPU::popFromStackImm16() {
        return _memory.readImm16(incGetRegister(ESP, 1));
    }

    uint32_t CPU::popFromStackImm32() {
        return _memory.readImm32(incGetRegister(ESP, 3));
    }

    void CPU::halt() {
        _isHalted = true;
    }

    bool CPU::isHalted() {
        return _isHalted;
    }

    bool CPU::longMode() {
        return false;
    }

}
