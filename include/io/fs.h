#pragma once

#include <string>
#include <cstdio>
#include <cstdint>
#include <vector>

namespace x86e::io {

    std::vector<uint8_t> readfile(const std::string& path);

}
