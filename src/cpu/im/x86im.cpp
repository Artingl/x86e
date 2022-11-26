#include "cpu/im/x86im.h"


namespace x86e::im {

    InstructionsManager::InstructionsManager(cpu::CPU* cpu)
        : _cpu(cpu) {
    }

    InstructionsManager::~InstructionsManager() {
    }
}
