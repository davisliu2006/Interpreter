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

    inline bool is_opening_bracket(char c) {
        return c == '(' || c == '[' || c == '{';
    }
    inline bool is_closing_bracket(char c) {
        return c == ')' || c == ']' || c == '}';
    }

    inline string format_string(const string& str) {
        string val;
        int nesting = 0;
        for (int i = 0; i < str.size(); i++) {
            char c = str[i];
            char c0 = i > 0? str[i-1] : '\0';
            if (is_opening_bracket(c)) {nesting++;}
            else if (is_closing_bracket(c)) {nesting--;}
            if (c0 == '\n') {
                for (int i = 0; i < nesting; i++) {val += "  ";}
            }
            val += c;
        }
        return val;
    }
}