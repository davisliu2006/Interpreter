#pragma once

#include "ast.hpp"

namespace compiler {
     struct StackBlock {
        struct entry {
            ast::var_decl* var;
            int size;
            int offset;
            entry() {}
            entry(ast::var_decl* var, int size, int offset): var(var), size(size), offset(offset) {}
        };

        vector<entry> layout;
        
        void add_var(ast::var_decl* var) {
            int size = 8; // TODO: change this later
            int offset = (layout.empty()? 0 : layout.back().offset+layout.back().size);
            layout.push_back({var, size, offset});
        }
    };
    inline std::ostream& operator<<(std::ostream& out, const StackBlock& stack_block) {
        out << "StackBlock {\n";
        for (const StackBlock::entry& entry: stack_block.layout) {
            out << "  " << entry.var->var << " (size: " << entry.size << ", offset: " << entry.offset << ")\n";
        }
        out << "}";
        return out;
    }

    using StackBlockMap = unordered_map<ast::stmt*,StackBlock>;
}