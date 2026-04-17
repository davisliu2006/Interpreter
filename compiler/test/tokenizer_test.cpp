#include <cassert>
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
        for (int i = 0; i < tokens.size(); i++) {
            assert(tokens[i].type != sym_t::ID);
        }
    }),
    Test("if", []() {
        auto tokens = tokenizer::tokenize("if (x == 0) {y = 1;}");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
        assert(tokens.size() == 12);
        assert(tokens[0].type == sym_t::IF);
        assert(tokens[1].type == sym_t::ROUND_LEFT);
        assert(tokens[2].type == sym_t::ID);
        assert(tokens[3].type == sym_t::OPERATOR);
        assert(tokens[4].type == sym_t::INT);
        assert(tokens[5].type == sym_t::ROUND_RIGHT);
        assert(tokens[6].type == sym_t::CURLY_LEFT);
        assert(tokens[7].type == sym_t::ID);
        assert(tokens[8].type == sym_t::OPERATOR);
        assert(tokens[9].type == sym_t::INT);
        assert(tokens[10].type == sym_t::SEMI);
        assert(tokens[11].type == sym_t::CURLY_RIGHT);
    }),
    Test("for", []() {
        auto tokens = tokenizer::tokenize("for (int i = 0; i < 10; i++) {y = 1;}");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
        assert(tokens[0].type == sym_t::FOR);
        assert(tokens[1].type == sym_t::ROUND_LEFT);
        assert(tokens[2].type == sym_t::TYPE);
        assert(tokens[3].type == sym_t::ID);
        assert(tokens[4].type == sym_t::OPERATOR);
        assert(tokens[5].type == sym_t::INT);
        assert(tokens[6].type == sym_t::SEMI);
        assert(tokens[7].type == sym_t::ID);
        assert(tokens[8].type == sym_t::OPERATOR);
        assert(tokens[9].type == sym_t::INT);
        assert(tokens[10].type == sym_t::SEMI);
        assert(tokens[11].type == sym_t::ID);
        assert(tokens[12].type == sym_t::OPERATOR);
        assert(tokens[13].type == sym_t::ROUND_RIGHT);
    }),
    Test("while", []() {
        auto tokens = tokenizer::tokenize("while (x == 0) {y = 1;}");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
        assert(tokens[0].type == sym_t::WHILE);
        assert(tokens[1].type == sym_t::ROUND_LEFT);
        assert(tokens[2].type == sym_t::ID);
        assert(tokens[3].type == sym_t::OPERATOR);
        assert(tokens[4].type == sym_t::INT);
        assert(tokens[5].type == sym_t::ROUND_RIGHT);
    }),
    Test("strings", []() {
        auto tokens = tokenizer::tokenize("string s = \"hi\"+'\\n';");
        for (const auto& [type, token]: tokens) {
            cout << "    " << type << ' ' << token << '\n';
        }
        assert(tokens[1].type == sym_t::ID);
        assert(tokens[2].type == sym_t::OPERATOR);
        assert(tokens[3].type == sym_t::STRING && tokens[3].text == "\"hi\"");
        assert(tokens[4].type == sym_t::OPERATOR);
        assert(tokens[5].type == sym_t::CHAR && tokens[5].text == "'\\n'");
        assert(tokens[6].type == sym_t::SEMI);
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