#include "io/fs.h"

namespace x86e::io {

    std::vector<uint8_t> readfile(const std::string& path) {
        std::vector<uint8_t> data;
        FILE* f = fopen(path.c_str(), "rb+");

        fseek(f, 0, SEEK_END);
        size_t fileSize = ftell(f);
        rewind(f);

        for (size_t i = 0; i < fileSize; i++)
            data.push_back(0);

        fread(&data[0], sizeof(uint8_t), fileSize, f);
        fclose(f);

        return data;
    }

}
