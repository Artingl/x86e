#pragma once

#include <cstdint>

namespace x86e::memory {
    class Memory {
    public:
        Memory(uint64_t size);
        ~Memory();

        uint8_t readImm8(uint64_t address);
        uint16_t readImm16(uint64_t address);
        uint32_t readImm32(uint64_t address);
        void writeImm8(uint8_t val, uint64_t address);
        void writeImm16(uint16_t val, uint64_t address);
        void writeImm32(uint32_t val, uint64_t address);
        void *getMemLocation();
        uint64_t memorySize();

    private:
        uint8_t* _memory;
        uint64_t _size;

    };

}