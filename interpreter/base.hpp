#pragma once

#include <array>
#include <cstdint>

#include "../compiler/base.hpp"

namespace interpreter {
    using std::pair, std::string, std::array, std::vector;
    using std::set, std::map, std::unordered_set, std::unordered_map;
    using std::cin, std::cout;

    enum class reg_t: uint8_t;

    enum class inst_t: uint8_t;
    struct inst;
}