#include "memory/memory.h"
#include "io/Logger.h"

namespace x86e::memory {

    Memory::Memory(uint64_t size) {
        x86e::io::debug_print(x86e::io::INFO, "Allocating %llu bytes for memory", size);

        _memory = new uint8_t[size];
        _size = size;
    }

    Memory::~Memory() {
        x86e::io::debug_print(x86e::io::INFO, "Deallocating memory");

        delete[] _memory;
    }

    void *Memory::getMemLocation() {
        return (void*)_memory;
    }

    uint8_t Memory::readImm8(uint64_t address) {
        return _memory[address];
    }

    uint16_t Memory::readImm16(uint64_t address) {
        return (uint16_t(_memory[address + 1]) << 8) | uint16_t(_memory[address]);
    }

    uint32_t Memory::readImm32(uint64_t address) {
        return (uint32_t(_memory[address + 3]) << 24) | (uint32_t(_memory[address + 2]) << 16) | (uint32_t(_memory[address + 1]) << 8) | uint32_t(_memory[address]);
    }

    void Memory::writeImm8(uint8_t val, uint64_t address) {
        _memory[address] = val;
    }

    void Memory::writeImm16(uint16_t val, uint64_t address) {
        _memory[address] = ((uint16_t)val >> 0) & 0xFF;
        _memory[address + 1] = ((uint16_t)val >> 8) & 0xFF;
    }

    void Memory::writeImm32(uint32_t val, uint64_t address) {
        _memory[address] = ((uint32_t)val >> 0) & 0xFF;
        _memory[address + 1] = ((uint32_t)val >> 8) & 0xFF;
        _memory[address + 2] = ((uint32_t)val >> 16) & 0xFF;
        _memory[address + 3] = ((uint32_t)val >> 24) & 0xFF;
    }

    uint64_t Memory::memorySize() {
        return _size;
    }

}
