#include <iomanip>
#include <iostream>

#include "test.hpp"
#include "../tokenizer.hpp"

using namespace std;
using namespace compiler;

vector<Test> tests = {
    Test("basic", []() {
        auto tokens = tokenizer::tokenize("int x = x0 0++%--x 0.11 0x1;");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
        assert(tokens.size() == 12);
        assert(tokens[0].type == sym_t::TYPE && tokens[0].text == "int");
        assert(tokens[1].type == sym_t::ID && tokens[1].text == "x");
        assert(tokens[2].type == sym_t::OPERATOR && tokens[2].text == "=");
        assert(tokens[3].type == sym_t::ID && tokens[3].text == "x0");
        assert(tokens[4].type == sym_t::INT && tokens[4].text == "0");
        assert(tokens[5].type == sym_t::OPERATOR && tokens[5].text == "++");
        assert(tokens[6].type == sym_t::OPERATOR && tokens[6].text == "%");
        assert(tokens[7].type == sym_t::OPERATOR && tokens[7].text == "--");
        assert(tokens[8].type == sym_t::ID && tokens[8].text == "x");
        assert(tokens[9].type == sym_t::FLOAT && tokens[9].text == "0.11");
        assert(tokens[10].type == sym_t::INT && tokens[10].text == "0x1");
        assert(tokens[11].type == sym_t::SEMI && tokens[11].text == ";");
    }),
    Test("comments", []() {
        auto tokens = tokenizer::tokenize("int x = 0; // abc def\nint y = 1; // ghi");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
        assert(tokens.size() == 12);
        for (int i = 0; i < 5; i++) {
            assert(tokens[i].type != sym_t::INVALID && tokens[i].type != sym_t::COMMENT);
        }
        assert(tokens[5].type == sym_t::COMMENT);
        for (int i = 6; i < 11; i++) {
            assert(tokens[i].type != sym_t::INVALID && tokens[i].type != sym_t::COMMENT);
        }
        assert(tokens[11].type == sym_t::COMMENT);
    }),
    Test("keywords", []() {
        auto tokens = tokenizer::tokenize("if else for while return in");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
    }),
    Test("if", []() {
        auto tokens = tokenizer::tokenize("if (x == 0) {y = 1;}");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
    }),
    Test("for", []() {
        auto tokens = tokenizer::tokenize("for (int i = 0; i < 10; i++) {y = 1;}");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
    }),
    Test("while", []() {
        auto tokens = tokenizer::tokenize("while (x == 0) {y = 1;}");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
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