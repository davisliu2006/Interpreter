#pragma once

#include "tokenizer.hpp"
#include "parser.hpp"
#include "resolver.hpp"
#include "code_generator.hpp"
#include "../interpreter/instructions.hpp"

namespace compiler {
    using inst = interpreter::inst;

    inline void init() {
        tokenizer::init();
    }

    inline vector<inst> compile(const string& code) {
        double t0 = time();
        auto tokens = tokenizer::tokenize(code);
        Parser parser(tokens);
        auto* ast = parser.parse_block(0, tokens.size());
        Resolver resolver(ast);
        resolver.resolve();
        CodeGenerator code_generator(ast, resolver.stack_blocks, resolver.f_index);
        auto insts = code_generator.generate();
        delete ast;
        double t1 = time();
        cout << "Compiled in: " << (t1-t0)*1000 << "ms\n";
        return insts;
    }
}