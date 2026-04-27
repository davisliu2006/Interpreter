#pragma once

#include "ast.hpp"
#include <cstddef>

namespace compiler {
    struct StackBlock {
        static const int PREFIX = 8;
        
        struct entry {
            ast::var_decl* var;
            int size;
            int offset;
            entry() {}
            entry(ast::var_decl* var, int size, int offset): var(var), size(size), offset(offset) {}
        };

        vector<entry> layout;

        constexpr int RA() {return 0;}
        void add_var(ast::var_decl* var) {
            int size = 8; // TODO: change this later
            int offset = (layout.empty()? PREFIX : layout.back().offset+layout.back().size);
            layout.push_back({var, size, offset});
        }
        entry* find_var(ast::var_decl* var) {
            for (entry& entry: layout) {
                if (entry.var == var) {return &entry;}
            }
            return NULL;
        }
        int size() const {
            return layout.empty()? 0 : layout.back().offset+layout.back().size;
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

    struct StackModel {
        vector<StackBlock> stack_model;

        StackBlock& operator[](int i) {return stack_model[i];}
        StackBlock& top() {return stack_model.back();}
        int size() const {return stack_model.size();}

        void push_block(const StackBlock& stack_block) {
            stack_model.push_back(stack_block);
        }
        void pop_block() {
            stack_model.pop_back();
        }
        int get_tot_offset(ast::var_decl* var) {
            int offset = 0;
            for (auto it = stack_model.rbegin(); it != stack_model.rend(); it++) {
                auto* entry = it->find_var(var);
                if (entry) {return offset+entry->offset;}
                offset += it->size();
            }
            throw std::runtime_error("Variable not found in stack model: "+string(var->var));
        }
    };

    using StackBlockMap = unordered_map<ast::stmt*,StackBlock>;

    struct IfAddr {
        size_t cond_offset;
        size_t cond_jump_offset;
        size_t end_jump_offset;

        IfAddr() {}
        IfAddr(size_t cond_offset, size_t cond_jump_offset, size_t end_jump_offset):
        cond_offset(cond_offset), cond_jump_offset(cond_jump_offset), end_jump_offset(end_jump_offset) {}
    };

    struct ForAddr {
        size_t cond_offset;
        size_t cond_jump_offset;
        size_t end_jump_offset;

        ForAddr() {}
    };

    struct WhileAddr {
        size_t cond_offset;
        size_t cond_jump_offset;
        size_t end_jump_offset;

        WhileAddr() {}
    };
}