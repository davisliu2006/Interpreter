#include <cassert>
#include <climits>
#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../tokenizer.hpp"
#include "../parser.hpp"

using namespace std;
using namespace compiler;

string display_str(int x) {
    if (x == INT_MAX) {return "INT_MAX";}
    else {return to_string(x);}
}

vector<Test> tests = {
    Test("brackets", []() {
        Parser parser(tokenizer::tokenize(";[(1+2),{3,4,[;;]}],({5,6});"));
        // --------------------------------0123456789012345678901234567
        // --------------------------------0         1         2
        cout << "    ";
        for (int i = 0; i < parser.closing_bracket.size(); i++) {
            cout << display_str(parser.closing_bracket[i]) << ' ';
        }
        cout << "\n    ";
        for (int i = 0; i < parser.next_comma.size(); i++) {
            cout << display_str(parser.next_comma[i]) << ' ';
        }
        cout << "\n    ";;
        for (int i = 0; i < parser.next_semicolon.size(); i++) {
            cout << display_str(parser.next_semicolon[i]) << ' ';
        }
        cout << '\n';
        vector<int> expected_closing_bracket = vector<int>{
            INT_MAX, 18, 6, 6, 6, 6, 6, 18,
            17, 17, 17, 17, 17, 16, 16, 16, 16, 17, 18, INT_MAX,
            26, 25, 25, 25, 25, 25, 26, INT_MAX
        };
        vector<int> expected_next_comma = vector<int>{
            19, 7, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 7,
            10, 10, 10, 12, 12, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX,
            19, INT_MAX, 23, 23, 23, INT_MAX, INT_MAX, INT_MAX, INT_MAX
        };
        vector<int> expected_next_semicolon = vector<int>{
            0, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX,
            INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 14, 14, 15, INT_MAX, INT_MAX, INT_MAX, 27,
            INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX, 27
        };
        assert(parser.closing_bracket == expected_closing_bracket);
        assert(parser.next_comma == expected_next_comma);
        assert(parser.next_semicolon == expected_next_semicolon);
    }),
    Test("while", []() {
        try {
            Parser parser(tokenizer::tokenize("while (x == 0) {y = 1;}"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
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
            cout << ast->to_formatted_string() << '\n';
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
            cout << ast->to_formatted_string() << '\n';
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
            cout << ast->to_formatted_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("f_call", []() {
        try {
            Parser parser(tokenizer::tokenize("f(1+2, 3, abc)"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("var_decl", []() {
        try {
            Parser parser(tokenizer::tokenize("int xyz"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("expr_binary", []() {
        try {
            Parser parser(tokenizer::tokenize("(a+b)*(c+d)+f/(g-h)"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("expr_with_f_call", []() {
        try {
            Parser parser(tokenizer::tokenize("f(a+b, c*d) + g(e)/f"));
            auto* ast = parser.parse_stmt(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
            delete ast;
        } catch (const exception& e) {
            cout << e.what() << '\n';
            throw e;
        }
    }),
    Test("block_simple", []() {
        try {
            Parser parser(tokenizer::tokenize("int x = 1; int y = 2;"));
            auto* ast = parser.parse_block(0, parser.tokens.size());
            cout << ast->to_formatted_string() << '\n';
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