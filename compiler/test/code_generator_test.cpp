#include <cassert>
#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../tokenizer.hpp"
#include "../parser.hpp"
#include "../resolver.hpp"
#include "../code_generator.hpp"

using namespace std;
using namespace compiler;

vector<Test> tests = {
    Test("if", []() {
        try {
            string code =
                "int x = 0;\n"
                "if (x == 0) {\n"
                "    x = 1;\n"
                "}\n"
            ;
            Parser parser(tokenizer::tokenize(code));
            auto* ast = parser.parse_block(0, parser.tokens.size());
            Resolver resolver(ast);
            cout << ast->to_formatted_string() << '\n';
            resolver.resolve();
            CodeGenerator generator(ast, resolver.stack_blocks, resolver.f_index);
            generator.generate();
            for (const interpreter::inst& inst: generator.main_insts) {
                cout << inst.to_string() << '\n';
            }
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("op_asn", []() {
        try {
            string code =
                "int x = 3;\n"
                "x++;\n"
                "x--;\n"
            ;
            Parser parser(tokenizer::tokenize(code));
            auto* ast = parser.parse_block(0, parser.tokens.size());
            Resolver resolver(ast);
            resolver.resolve();
            CodeGenerator generator(ast, resolver.stack_blocks, resolver.f_index);
            generator.generate();
            for (const interpreter::inst& inst: generator.main_insts) {
                cout << inst.to_string() << '\n';
            }
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("op_un", []() {
        try {
            string code =
                "int x = 3;\n"
                "int y = -x + ~x;\n"
                "int z = !x;\n"
            ;
            Parser parser(tokenizer::tokenize(code));
            auto* ast = parser.parse_block(0, parser.tokens.size());
            Resolver resolver(ast);
            resolver.resolve();
            CodeGenerator generator(ast, resolver.stack_blocks, resolver.f_index);
            generator.generate();
            for (const interpreter::inst& inst: generator.main_insts) {
                cout << inst.to_string() << '\n';
            }
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("f_def_f_call_early_return", []() {
        try {
            string code =
                "int f(int x) {\n"
                "    if (x < 0) {\n"
                "        return -1;\n"
                "    }\n"
                "    return 1;\n"
                "}\n"
                "f(0);\n"
            ;
            Parser parser(tokenizer::tokenize(code));
            auto* ast = parser.parse_block(0, parser.tokens.size());
            Resolver resolver(ast);
            resolver.resolve();
            CodeGenerator generator(ast, resolver.stack_blocks, resolver.f_index);
            generator.generate();
            bool has_call = false;
            for (const interpreter::inst& inst: generator.main_insts) {
                cout << inst.to_string() << '\n';
                if (inst.type() == interpreter::inst_t::call) {
                    has_call = true;
                }
            }
            assert(has_call);
            assert(generator.func_insts.size() == 1);
            assert(!generator.func_insts[0].empty());
            for (const interpreter::inst& inst: generator.func_insts[0]) {
                cout << "f0: " << inst.to_string() << '\n';
            }
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    cout << setprecision(15);

    tokenizer::init();

    for (const Test& test: tests) {
        test.run();
    }

    return 0;
}