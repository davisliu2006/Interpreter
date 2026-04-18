#pragma once

#include <cstdint>

#include "../compiler/base.hpp"

namespace interpreter {
    enum class reg_t: uint8_t;

    enum class inst_t: uint8_t;
    struct inst;
}