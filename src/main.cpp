#include <map>
#include "io/Logger.h"
#include "io/fs.h"
#include "cpu/i386.h"

#define MEM_SIZE 0xFFFFF /* in bytes */

using namespace x86e;

// for debugging
std::map<cpu::Registers, std::string> cpuRegistersMap = {
        { cpu::Registers::EAX, "EAX" },
        { cpu::Registers::ECX, "ECX" },
        { cpu::Registers::EDX, "EDX" },
        { cpu::Registers::EBX, "EBX" },
        { cpu::Registers::ESP, "ESP" },
        { cpu::Registers::EBP, "EBP" },
        { cpu::Registers::ESI, "ESI" },
        { cpu::Registers::EDI, "EDI" },
        { cpu::Registers::EIP, "EIP" },
        { cpu::Registers::AX, "AX" },
        { cpu::Registers::CX, "CX" },
        { cpu::Registers::DX, "DX" },
        { cpu::Registers::BX, "BX" },
        { cpu::Registers::AL, "AL" },
        { cpu::Registers::CL, "CL" },
        { cpu::Registers::DL, "DL" },
        { cpu::Registers::BL, "BL" },
        { cpu::Registers::AH, "AH" },
        { cpu::Registers::CH, "CH" },
        { cpu::Registers::DH, "DH" },
        { cpu::Registers::BH, "BH" },
        { cpu::Registers::CS, "CS" },
        { cpu::Registers::DS, "DS" },
        { cpu::Registers::ES, "ES" },
        { cpu::Registers::FS, "FS" },
        { cpu::Registers::GS, "GS" },
        { cpu::Registers::SS, "SS" },
        { cpu::Registers::SP, "SP" },
        { cpu::Registers::BP, "BP" },
        { cpu::Registers::SI, "SI" },
        { cpu::Registers::DI, "DI" },
        { cpu::Registers::IP, "IP" },
};

std::map<cpu::Flags, std::string> cpuFlagsMap = {
        { cpu::Flags::CF, "CF" },
        { cpu::Flags::PF, "PF" },
        { cpu::Flags::AF, "AF" },
        { cpu::Flags::ZF, "ZF" },
        { cpu::Flags::SF, "SF" },
        { cpu::Flags::TF, "TF" },
        { cpu::Flags::IF, "IF" },
        { cpu::Flags::DF, "DF" },
        { cpu::Flags::OF, "OF" },
        { cpu::Flags::IOPL, "IOPL" },
        { cpu::Flags::NT, "NT" },
        { cpu::Flags::RF, "RF" },
        { cpu::Flags::VM, "VM" },
};

int main() {
    io::debug_print(io::INFO, "x86e v%s", VERSION);

    cpu::i386 cpu(MEM_SIZE);
    cpu.reset();

    std::vector<uint8_t> data = io::readfile("../stuff/main");

    size_t offset = 0;
    for (uint8_t byte : data) {
        cpu.getMemory().writeImm8(byte, cpu.getRegister(x86e::cpu::EIP) + (offset++));
    }

    cpu.getMemory().writeImm32(0xaafa113, 0xfa);

    while (!cpu.isHalted()) {
        cpu.cycle();
        io::debug_print(x86e::io::INFO, "CPU status:");

        for ( const auto& a : cpuRegistersMap )
        {
            printf("\t- %s: 0x%x\n", a.second.c_str(), cpu.getRegister(a.first));
        }

        printf("\t- ");
        size_t f = 0;

        for ( const auto& a : cpuFlagsMap )
        {
            printf("%s: 0x%x | ", a.second.c_str(), cpu.getFlag(a.first));

            if (++f > 4) {
                printf("\n\t- ");
                f = 0;
            }
        }

        printf("\n\t0xFA addr: 0x%x\n", cpu.getMemory().readImm32(0xfa));
    }

}
