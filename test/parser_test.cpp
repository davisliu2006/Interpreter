#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../parser.hpp"

using namespace std;
using namespace compiler;

vector<Test> tests = {
    Test("brackets", []() {
        Parser parser(tokenizer::tokenize("[(1+2){[3,4]}]({5,6});"));
        // --------------------------------0123456789012345678901
        // --------------------------------0         1         2
        cout << "    ";
        for (int i = 0; i < parser.tokens.size(); i++) {
            cout << parser.closing_bracket[i] << ' ';
        }
        cout << '\n';
        vector<int> expected = vector<int>{
            13, 5, 5, 5, 5, 5, 12, 11, 11, 11, 11, 11, 12, 13, 20, 19, 19, 19, 19, 19, 20, INT_MAX
        };
        assert(parser.closing_bracket == expected);
    }),
    Test("while", []() {
        try {
            Parser parser(tokenizer::tokenize("while (x == 0) {y = 1;}"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("for", []() {
        try {
            Parser parser(tokenizer::tokenize("for (int i = 0; i < 10; i++) {}"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("if_fallback", []() {
        try {
            Parser parser(tokenizer::tokenize("if (x == 0) {y = 1;}\nelse if (x == 1) {y = 2;}\nelse {y = 3;}"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("if_no_fallback", []() {
        try {
            Parser parser(tokenizer::tokenize("if (x == 0) {y = 1;}\nelse if (x == 1) {y = 2;}\nelse if (x == 2) {y = 3;}"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("f_call", []() {
        try {
            Parser parser(tokenizer::tokenize("f(1+2, 3, abc);"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("var_decl", []() {
        try {
            Parser parser(tokenizer::tokenize("int xyz;"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    })
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