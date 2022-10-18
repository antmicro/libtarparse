#pragma once
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

namespace fileio {
    inline std::optional<std::vector<uint8_t>> load_file(std::string file)
    {
        std::FILE* p = std::fopen(file.c_str(), "rb");
        if(!p) {
            perror("std::fopen");
            return {};
        }

        std::fseek(p, 0, SEEK_END);
        const size_t size = std::ftell(p);
        std::fseek(p, 0, SEEK_SET);

        std::vector<uint8_t> vec {};
        vec.resize(size);

        std::fread(vec.data(), sizeof(uint8_t), vec.size(), p);
        std::fclose(p);
        return { std::move(vec) };
    }
}
