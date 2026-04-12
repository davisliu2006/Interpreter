#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace compiler {
    using std::pair, std::string, std::array, std::vector;
    using std::set, std::map, std::unordered_set, std::unordered_map;
    using std::cin, std::cout;

    template <class type>
    inline bool in_range(type x, type l, type r) {
        return l <= x && x <= r;
    }
}