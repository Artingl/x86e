#include "cpu/im/i386im.h"

#include <algorithm>


namespace x86e::im {

    i386_InstructionsManager::i386_InstructionsManager(cpu::CPU* cpu)
            : InstructionsManager(cpu) {
    }

    i386_InstructionsManager::~i386_InstructionsManager() {
    }


    REF_INSTRUCTION(i386_InstructionsManager, add_rm8_r8) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false);
            secondRegister = _cpu->ModRMValue32bit(opcode, true);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false);
            secondRegister = _cpu->ModRMValue16bit(opcode, true);
        }

        uint8_t fResult;
        uint8_t sResult;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            uint16_t addr = firstRegister;
            fResult = _cpu->getMemory().readImm8(addr);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->getMemory().writeImm8(fResult + sResult, addr);
        }
        else if (opcode.mod_or_index == 0b11) {
            fResult = _cpu->getRegister((cpu::Registers)firstRegister);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers)firstRegister, fResult + sResult);
        }

        if ((uint8_t)(fResult + sResult) < fResult) {
            // Overflow
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult + sResult)) >> 7);

        if (_cpu->getFlag(cpu::CF) && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult + sResult)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult + sResult))&0xff) % 2 == 0);
        _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult&0xF)+(uint8_t)(sResult&0xF)) > 15);
    }

    REF_INSTRUCTION(i386_InstructionsManager, add_rm16_32_r16_32) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint8_t offset = 1;
        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            ++offset;
        }

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue32bit(opcode, true, offset);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue16bit(opcode, true, offset);
        }

        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            if (offset == 1) { // 16 bit
                uint16_t addr = firstRegister;
                fResult16 = _cpu->getMemory().readImm16(addr);
                sResult16 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->getMemory().writeImm16(fResult16 + sResult16, addr);
            }
            else { // 32 bit
                uint32_t addr = firstRegister;
                fResult32 = _cpu->getMemory().readImm32(addr);
                sResult32 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->getMemory().writeImm32(fResult32 + sResult32, addr);
            }
        }
        else if (opcode.mod_or_index == 0b11) {
            if (offset == 1) { // 16 bit
                fResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult16 + sResult16);
            }
            else { // 32 bit
                fResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult32 + sResult32);
            }
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 + sResult16) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 + sResult16)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 + sResult16)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 + sResult16))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult16&0xF)+(uint8_t)(sResult16&0xF)) > 15);
        }
        else {
            if ((uint32_t)(fResult32 + sResult32) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 + sResult32)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 + sResult32)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 + sResult32))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult32&0xF)+(uint8_t)(sResult32&0xF)) > 15);
        }

        if (_cpu->getFlag(cpu::CF) && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }
    }


    REF_INSTRUCTION(i386_InstructionsManager, add_r8_rm8) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false);
            secondRegister = _cpu->ModRMValue32bit(opcode, true);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false);
            secondRegister = _cpu->ModRMValue16bit(opcode, true);
        }

        uint8_t fResult;
        uint8_t sResult;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            uint16_t addr = firstRegister;
            fResult = _cpu->getMemory().readImm8(addr);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers) secondRegister, fResult + sResult);
        }
        else if (opcode.mod_or_index == 0b11) {
            fResult = _cpu->getRegister((cpu::Registers)firstRegister);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers)firstRegister, fResult + sResult);
        }

        if ((uint8_t)(fResult + sResult) < fResult) {
            // Overflow
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult + sResult)) >> 7);

        if (_cpu->getFlag(cpu::CF) && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult + sResult)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult + sResult))&0xff) % 2 == 0);
        _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult&0xF)+(uint8_t)(sResult&0xF)) > 15);
    }

    REF_INSTRUCTION(i386_InstructionsManager, add_r16_32_rm16_32) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint8_t offset = 1;
        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            ++offset;
        }

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue32bit(opcode, true, offset);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue16bit(opcode, true, offset);
        }

        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            if (offset == 1) { // 16 bit
                uint16_t addr = firstRegister;
                fResult16 = _cpu->getMemory().readImm16(addr);
                sResult16 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->setRegister((cpu::Registers) secondRegister, fResult16 + sResult16);
            }
            else { // 32 bit
                uint32_t addr = firstRegister;
                fResult32 = _cpu->getMemory().readImm32(addr);
                sResult32 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->setRegister((cpu::Registers) secondRegister, fResult32 + sResult32);
            }
        }
        else if (opcode.mod_or_index == 0b11) {
            if (offset == 1) { // 16 bit
                fResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult16 + sResult16);
            }
            else { // 32 bit
                fResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult32 + sResult32);
            }
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 + sResult16) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 + sResult16)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 + sResult16)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 + sResult16))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult16&0xF)+(uint8_t)(sResult16&0xF)) > 15);
        }
        else {
            if ((uint32_t)(fResult32 + sResult32) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 + sResult32)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 + sResult32)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 + sResult32))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult32&0xF)+(uint8_t)(sResult32&0xF)) > 15);
        }

        if (_cpu->getFlag(cpu::CF) && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }
    }

    REF_INSTRUCTION(i386_InstructionsManager, add_al_imm8) {
        uint8_t fResult;
        uint8_t sResult;

        fResult = _cpu->getRegister(cpu::Registers::AL);
        sResult = _cpu->getMemory().readImm8(_cpu->incGetRegister(cpu::Registers::EIP));

        _cpu->setRegister(cpu::Registers::AL,fResult + sResult);

        if ((uint8_t)(fResult + sResult) < fResult) {
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult + sResult)) >> 7);

        if (_cpu->getFlag(cpu::CF) && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult + sResult)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult + sResult))&0xff) % 2 == 0);
        _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult&0xF)+(uint8_t)(sResult&0xF)) > 15);
    }

    REF_INSTRUCTION(i386_InstructionsManager, add_eAX_imm16_32) {
        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        uint8_t offset;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            fResult32 = (uint32_t)_cpu->getRegister(cpu::Registers::EAX);
            sResult32 = (uint32_t)_cpu->getMemory().readImm32(_cpu->incGetRegister(cpu::Registers::EIP, 4));
            _cpu->setRegister(cpu::Registers::EAX,fResult32 + sResult32);

            offset = 2;
        }
        else {
            offset = 1;

            fResult16 = (uint32_t)_cpu->getRegister(cpu::Registers::AX);
            sResult16 = (uint32_t)_cpu->getMemory().readImm16(_cpu->incGetRegister(cpu::Registers::EIP, 2));
            _cpu->setRegister(cpu::Registers::AX,fResult16 + sResult16);
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 + sResult16) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 + sResult16)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 + sResult16)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 + sResult16))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult16&0xF)+(uint8_t)(sResult16&0xF)) > 15);
        }
        else {
            if ((uint32_t)(fResult32 + sResult32) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 + sResult32)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 + sResult32)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 + sResult32))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult32&0xF)+(uint8_t)(sResult32&0xF)) > 15);
        }

        if (_cpu->getFlag(cpu::CF) && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }
    }

    REF_INSTRUCTION(i386_InstructionsManager, halt) {
        _cpu->halt();
    }

    REF_INSTRUCTION(i386_InstructionsManager, push_es) {
        _cpu->pushOntoStackImm16(_cpu->getRegister(cpu::Registers::ES));
    }

    REF_INSTRUCTION(i386_InstructionsManager, pop_es) {
        _cpu->setRegister(cpu::Registers::ES, _cpu->popFromStackImm16());
    }

    REF_INSTRUCTION(i386_InstructionsManager, push_cs) {
        _cpu->pushOntoStackImm16(_cpu->getRegister(cpu::Registers::CS));
    }

    REF_INSTRUCTION(i386_InstructionsManager, or_rm8_r8) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false);
            secondRegister = _cpu->ModRMValue32bit(opcode, true);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false);
            secondRegister = _cpu->ModRMValue16bit(opcode, true);
        }

        uint8_t fResult;
        uint8_t sResult;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            uint16_t addr = firstRegister;
            fResult = _cpu->getMemory().readImm8(addr);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->getMemory().writeImm8(fResult | sResult, addr);
        }
        else if (opcode.mod_or_index == 0b11) {
            fResult = _cpu->getRegister((cpu::Registers)firstRegister);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers)firstRegister, fResult | sResult);
        }

        if ((uint8_t)(fResult | sResult) < fResult) {
            // Overflow
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult | sResult)) >> 7);

        _cpu->setFlag(cpu::CF, 0);
        _cpu->setFlag(cpu::OF, 0);

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult | sResult)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult | sResult))&0xff) % 2 == 0);
    }

    REF_INSTRUCTION(i386_InstructionsManager, or_rm16_32_r16_32) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint8_t offset = 1;
        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            ++offset;
        }

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue32bit(opcode, true, offset);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue16bit(opcode, true, offset);
        }

        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            if (offset == 1) { // 16 bit
                uint16_t addr = firstRegister;
                fResult16 = _cpu->getMemory().readImm16(addr);
                sResult16 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->getMemory().writeImm16(fResult16 | sResult16, addr);
            }
            else { // 32 bit
                uint32_t addr = firstRegister;
                fResult32 = _cpu->getMemory().readImm32(addr);
                sResult32 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->getMemory().writeImm32(fResult32 | sResult32, addr);
            }
        }
        else if (opcode.mod_or_index == 0b11) {
            if (offset == 1) { // 16 bit
                fResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult16 | sResult16);
            }
            else { // 32 bit
                fResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult32 | sResult32);
            }
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 | sResult16) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 | sResult16)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 | sResult16)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 | sResult16))&0xff) % 2 == 0);
        }
        else {
            if ((uint32_t)(fResult32 | sResult32) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 | sResult32)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 | sResult32)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 | sResult32))&0xff) % 2 == 0);
        }

        _cpu->setFlag(cpu::CF, 0);
        _cpu->setFlag(cpu::OF, 0);
    }


    REF_INSTRUCTION(i386_InstructionsManager, or_r8_rm8) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false);
            secondRegister = _cpu->ModRMValue32bit(opcode, true);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false);
            secondRegister = _cpu->ModRMValue16bit(opcode, true);
        }

        uint8_t fResult;
        uint8_t sResult;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            uint16_t addr = firstRegister;
            fResult = _cpu->getMemory().readImm8(addr);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers) secondRegister, fResult | sResult);
        }
        else if (opcode.mod_or_index == 0b11) {
            fResult = _cpu->getRegister((cpu::Registers)firstRegister);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers)firstRegister, fResult | sResult);
        }

        if ((uint8_t)(fResult | sResult) < fResult) {
            // Overflow
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult | sResult)) >> 7);

        _cpu->setFlag(cpu::CF, 0);
        _cpu->setFlag(cpu::OF, 0);

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult | sResult)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult | sResult))&0xff) % 2 == 0);
    }

    REF_INSTRUCTION(i386_InstructionsManager, or_r16_32_rm16_32) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint8_t offset = 1;
        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            ++offset;
        }

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue32bit(opcode, true, offset);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue16bit(opcode, true, offset);
        }

        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            if (offset == 1) { // 16 bit
                uint16_t addr = firstRegister;
                fResult16 = _cpu->getMemory().readImm16(addr);
                sResult16 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->setRegister((cpu::Registers) secondRegister, fResult16 | sResult16);
            }
            else { // 32 bit
                uint32_t addr = firstRegister;
                fResult32 = _cpu->getMemory().readImm32(addr);
                sResult32 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->setRegister((cpu::Registers) secondRegister, fResult32 | sResult32);
            }
        }
        else if (opcode.mod_or_index == 0b11) {
            if (offset == 1) { // 16 bit
                fResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult16 | sResult16);
            }
            else { // 32 bit
                fResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult32 | sResult32);
            }
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 | sResult16) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 | sResult16)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 | sResult16)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 | sResult16))&0xff) % 2 == 0);
        }
        else {
            if ((uint32_t)(fResult32 | sResult32) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 | sResult32)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 | sResult32)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 | sResult32))&0xff) % 2 == 0);
        }

        _cpu->setFlag(cpu::CF, 0);
        _cpu->setFlag(cpu::OF, 0);
    }

    REF_INSTRUCTION(i386_InstructionsManager, or_al_imm8) {
        uint8_t fResult;
        uint8_t sResult;

        fResult = _cpu->getRegister(cpu::Registers::AL);
        sResult = _cpu->getMemory().readImm8(_cpu->incGetRegister(cpu::Registers::EIP));

        _cpu->setRegister(cpu::Registers::AL,fResult | sResult);

        if ((uint8_t)(fResult | sResult) < fResult) {
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult | sResult)) >> 7);

        _cpu->setFlag(cpu::CF, 0);
        _cpu->setFlag(cpu::OF, 0);

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult | sResult)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult | sResult))&0xff) % 2 == 0);
    }

    REF_INSTRUCTION(i386_InstructionsManager, or_eAX_imm16_32) {
        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        uint8_t offset;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            fResult32 = (uint32_t)_cpu->getRegister(cpu::Registers::EAX);
            sResult32 = (uint32_t)_cpu->getMemory().readImm32(_cpu->incGetRegister(cpu::Registers::EIP, 4));
            _cpu->setRegister(cpu::Registers::EAX,fResult32 | sResult32);

            offset = 2;
        }
        else {
            offset = 1;

            fResult16 = (uint32_t)_cpu->getRegister(cpu::Registers::AX);
            sResult16 = (uint32_t)_cpu->getMemory().readImm16(_cpu->incGetRegister(cpu::Registers::EIP, 2));
            _cpu->setRegister(cpu::Registers::AX,fResult16 | sResult16);
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 | sResult16) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 | sResult16)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 | sResult16)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 | sResult16))&0xff) % 2 == 0);
        }
        else {
            if ((uint32_t)(fResult32 | sResult32) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 | sResult32)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 | sResult32)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 | sResult32))&0xff) % 2 == 0);
        }

        _cpu->setFlag(cpu::CF, 0);
        _cpu->setFlag(cpu::OF, 0);
    }

    REF_INSTRUCTION(i386_InstructionsManager, adc_rm8_r8) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false);
            secondRegister = _cpu->ModRMValue32bit(opcode, true);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false);
            secondRegister = _cpu->ModRMValue16bit(opcode, true);
        }

        uint8_t fResult;
        uint8_t sResult;

        uint8_t carryFlag = _cpu->getFlag(cpu::CF);

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            uint16_t addr = firstRegister;
            fResult = _cpu->getMemory().readImm8(addr);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->getMemory().writeImm8(fResult + sResult + carryFlag, addr);
        }
        else if (opcode.mod_or_index == 0b11) {
            fResult = _cpu->getRegister((cpu::Registers)firstRegister);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers)firstRegister, fResult + sResult + carryFlag);
        }

        if ((uint8_t)(fResult + sResult + carryFlag) < fResult) {
            // Overflow
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult + sResult + carryFlag)) >> 7);

        if (carryFlag && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult + sResult + carryFlag)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult + sResult + carryFlag))&0xff) % 2 == 0);
        _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult&0xF)+(uint8_t)(sResult&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
    }

    REF_INSTRUCTION(i386_InstructionsManager, adc_rm16_32_r16_32) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint8_t offset = 1;
        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            ++offset;
        }

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue32bit(opcode, true, offset);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue16bit(opcode, true, offset);
        }

        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        uint8_t carryFlag = _cpu->getFlag(cpu::CF);

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            if (offset == 1) { // 16 bit
                uint16_t addr = firstRegister;
                fResult16 = _cpu->getMemory().readImm16(addr);
                sResult16 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->getMemory().writeImm16(fResult16 + sResult16 + carryFlag, addr);
            }
            else { // 32 bit
                uint32_t addr = firstRegister;
                fResult32 = _cpu->getMemory().readImm32(addr);
                sResult32 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->getMemory().writeImm32(fResult32 + sResult32 + carryFlag, addr);
            }
        }
        else if (opcode.mod_or_index == 0b11) {
            if (offset == 1) { // 16 bit
                fResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult16 + sResult16 + carryFlag);
            }
            else { // 32 bit
                fResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult32 + sResult32 + carryFlag);
            }
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 + sResult16 + carryFlag) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 + sResult16 + carryFlag)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 + sResult16 + carryFlag)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 + sResult16 + carryFlag))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult16&0xF)+(uint8_t)(sResult16&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
        }
        else {
            if ((uint32_t)(fResult32 + sResult32 + carryFlag) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 + sResult32 + carryFlag)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 + sResult32 + carryFlag)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 + sResult32 + carryFlag))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult32&0xF)+(uint8_t)(sResult32&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
        }

        if (carryFlag && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }
    }


    REF_INSTRUCTION(i386_InstructionsManager, adc_r8_rm8) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false);
            secondRegister = _cpu->ModRMValue32bit(opcode, true);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false);
            secondRegister = _cpu->ModRMValue16bit(opcode, true);
        }

        uint8_t fResult;
        uint8_t sResult;

        uint8_t carryFlag = _cpu->getFlag(cpu::CF);

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            uint16_t addr = firstRegister;
            fResult = _cpu->getMemory().readImm8(addr);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers) secondRegister, fResult + sResult + carryFlag);
        }
        else if (opcode.mod_or_index == 0b11) {
            fResult = _cpu->getRegister((cpu::Registers)firstRegister);
            sResult = _cpu->getRegister((cpu::Registers)secondRegister);

            _cpu->setRegister((cpu::Registers)firstRegister, fResult + sResult + carryFlag);
        }

        if ((uint8_t)(fResult + sResult + carryFlag) < fResult) {
            // Overflow
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult + sResult + carryFlag)) >> 7);

        if (carryFlag && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult + sResult + carryFlag)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult + sResult + carryFlag))&0xff) % 2 == 0);
        _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult&0xF)+(uint8_t)(sResult&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
    }

    REF_INSTRUCTION(i386_InstructionsManager, adc_r16_32_rm16_32) {
        _cpu->parseModRM(opcode, _cpu->incGetRegister(cpu::Registers::EIP));

        uint8_t offset = 1;
        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            ++offset;
        }

        uint32_t firstRegister;
        uint32_t secondRegister;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::ADDRESS_SIZE)) {
            firstRegister = _cpu->ModRMValue32bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue32bit(opcode, true, offset);
        }
        else {
            firstRegister = _cpu->ModRMValue16bit(opcode, false, offset);
            secondRegister = _cpu->ModRMValue16bit(opcode, true, offset);
        }

        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        uint8_t carryFlag = _cpu->getFlag(cpu::CF);

        if (opcode.mod_or_index == 0b00 || opcode.mod_or_index == 0b01 || opcode.mod_or_index == 0b10) {
            if (offset == 1) { // 16 bit
                uint16_t addr = firstRegister;
                fResult16 = _cpu->getMemory().readImm16(addr);
                sResult16 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->setRegister((cpu::Registers) secondRegister, fResult16 + sResult16 + carryFlag);
            }
            else { // 32 bit
                uint32_t addr = firstRegister;
                fResult32 = _cpu->getMemory().readImm32(addr);
                sResult32 = _cpu->getRegister((cpu::Registers)secondRegister);

                _cpu->setRegister((cpu::Registers) secondRegister, fResult32 + sResult32 + carryFlag);
            }
        }
        else if (opcode.mod_or_index == 0b11) {
            if (offset == 1) { // 16 bit
                fResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult16 = (uint16_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult16 + sResult16 + carryFlag);
            }
            else { // 32 bit
                fResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) firstRegister);
                sResult32 = (uint32_t)_cpu->getRegister((cpu::Registers) secondRegister);

                _cpu->setRegister((cpu::Registers) firstRegister, fResult32 + sResult32 + carryFlag);
            }
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 + sResult16 + carryFlag) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 + sResult16 + carryFlag)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 + sResult16 + carryFlag)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 + sResult16 + carryFlag))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult16&0xF)+(uint8_t)(sResult16&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
        }
        else {
            if ((uint32_t)(fResult32 + sResult32 + carryFlag) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 + sResult32 + carryFlag)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 + sResult32 + carryFlag)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 + sResult32 + carryFlag))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult32&0xF)+(uint8_t)(sResult32&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
        }

        if (carryFlag && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }
    }

    REF_INSTRUCTION(i386_InstructionsManager, adc_al_imm8) {
        uint8_t fResult;
        uint8_t sResult;

        uint8_t carryFlag = _cpu->getFlag(cpu::CF);

        fResult = _cpu->getRegister(cpu::Registers::AL);
        sResult = _cpu->getMemory().readImm8(_cpu->incGetRegister(cpu::Registers::EIP));

        _cpu->setRegister(cpu::Registers::AL,fResult + sResult + carryFlag);

        if ((uint8_t)(fResult + sResult + carryFlag) < fResult) {
            _cpu->setFlag(cpu::CF, 1);
        }

        _cpu->setFlag(cpu::SF, ((uint8_t)(fResult + sResult + carryFlag)) >> 7);

        if (carryFlag && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }

        _cpu->setFlag(cpu::ZF, ((uint8_t)(fResult + sResult + carryFlag)) == 0);
        _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint8_t)(fResult + sResult + carryFlag))&0xff) % 2 == 0);
        _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult&0xF)+(uint8_t)(sResult&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
    }

    REF_INSTRUCTION(i386_InstructionsManager, adc_eAX_imm16_32) {
        uint16_t fResult16;
        uint16_t sResult16;

        uint32_t fResult32;
        uint32_t sResult32;

        uint8_t carryFlag = _cpu->getFlag(cpu::CF);

        uint8_t offset;

        if (OP_CHECK_PREFIX(opcode.prefixes, cpu::InstructionPrefix::OPERAND_SIZE) || _cpu->longMode()) {
            fResult32 = (uint32_t)_cpu->getRegister(cpu::Registers::EAX);
            sResult32 = (uint32_t)_cpu->getMemory().readImm32(_cpu->incGetRegister(cpu::Registers::EIP, 4));
            _cpu->setRegister(cpu::Registers::EAX,fResult32 + sResult32 + carryFlag);

            offset = 2;
        }
        else {
            offset = 1;

            fResult16 = (uint32_t)_cpu->getRegister(cpu::Registers::AX);
            sResult16 = (uint32_t)_cpu->getMemory().readImm16(_cpu->incGetRegister(cpu::Registers::EIP, 2));
            _cpu->setRegister(cpu::Registers::AX,fResult16 + sResult16 + carryFlag);
        }

        if (offset == 1) {
            if ((uint16_t)(fResult16 + sResult16 + carryFlag) < fResult16) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint16_t)(fResult16 + sResult16 + carryFlag)) >> 15);

            _cpu->setFlag(cpu::ZF, ((uint16_t)(fResult16 + sResult16 + carryFlag)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint16_t)(fResult16 + sResult16 + carryFlag))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult16&0xF)+(uint8_t)(sResult16&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
        }
        else {
            if ((uint32_t)(fResult32 + sResult32 + carryFlag) < fResult32) {
                // Overflow
                _cpu->setFlag(cpu::CF, 1);
            }

            _cpu->setFlag(cpu::SF, ((uint32_t)(fResult32 + sResult32 + carryFlag)) >> 31);

            _cpu->setFlag(cpu::ZF, ((uint32_t)(fResult32 + sResult32 + carryFlag)) == 0);
            _cpu->setFlag(cpu::PF, x86e::utils::countWithOddSetBits(((uint32_t)(fResult32 + sResult32 + carryFlag))&0xff) % 2 == 0);
            _cpu->setFlag(cpu::AF, (uint8_t)((uint8_t)(fResult32&0xF)+(uint8_t)(sResult32&0xF)+(uint8_t)(carryFlag&0xF)) > 15);
        }

        if (carryFlag && _cpu->getFlag(cpu::SF)) {
            _cpu->setFlag(cpu::CF, 0);
            _cpu->setFlag(cpu::OF, 1);
        }
    }

    REF_INSTRUCTION(i386_InstructionsManager, push_ss) {
        _cpu->pushOntoStackImm16(_cpu->getRegister(cpu::Registers::SS));
    }

    REF_INSTRUCTION(i386_InstructionsManager, pop_ss) {
        _cpu->setRegister(cpu::Registers::SS, _cpu->popFromStackImm16());
    }

}
