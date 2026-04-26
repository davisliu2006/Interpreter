#pragma once

#include "base.hpp"
#include "symbols.hpp"

namespace compiler {
    namespace tokenizer {
        inline void init() {
            symbols::init();
        }

        inline Token next_token(const char* substr) {
            if (char_type(*substr) == sym_t::WS) { // whitespace
                string text;
                while (char_type(*substr) == sym_t::WS) {
                    text += *substr;
                    substr++;
                }
                return {sym_t::WS, text};
            } else if (*substr == '"') { // string literal
                string text = "\"";
                substr++;
                bool escape = false;
                while (*substr && (*substr != '"' || escape)) {
                    text += *substr;
                    escape = (*substr == '\\');
                    substr++;
                }
                if (!substr) {return {sym_t::INVALID, text};}
                text += '\"';
                return {sym_t::STRING, text};
            } else if (*substr == '\'') { // char literal
                string text = "\'";
                substr++;
                if (*substr == '\\') {
                    text += *substr; substr++;
                }
                if (!*substr) {return {sym_t::INVALID, text};}
                text += *substr; substr++;
                if (*substr != '\'') {return {sym_t::INVALID, text};}
                text += '\'';
                return {sym_t::CHAR, text};
            } else if (char_type(*substr) == sym_t::OPERATOR) {
                if (substr[0] == '/' && substr[1] == '/') { // comment
                    string text = "//";
                    substr += 2;
                    while (*substr && *substr != '\n') {
                        text += *substr;
                        substr++;
                    }
                    return {sym_t::COMMENT, text};
                } else { // operator
                    auto [text, node] = op_char_symbols.get_longest_prefix(substr);
                    if (node) {
                        return {node->val, text};
                    }
                    return {sym_t::INVALID, ""};
                }
            } else {
                string text;
                if (in_range(*substr, '0', '9')) { // number
                    int state = 0b111;
                    if (substr[0] == '0' && substr[1] == 'x') {
                        state = 0b101;
                        text += "0x";
                        substr += 2;
                    }
                    while (char_type(*substr) == sym_t::ID || *substr == '.') {
                        if (*substr == '.') {
                            if (state & 0b100) {state &= 0b010;}
                            else {state &= 0b000;}
                        } else if (!in_range(*substr, '0', '9')) {
                            std::cerr << "Invalid @ " << *substr << '\n';
                            state = 0b000;
                        }
                        text += *substr;
                        substr++;
                    }
                    if (state & 0b001) {return {sym_t::INT, text};}
                    else if (state & 0b010) {return {sym_t::FLOAT, text};}
                    else {return {sym_t::INVALID, text};}
                } else { // identifier, keyword, or type
                    while (char_type(*substr) == sym_t::ID) {
                        text += *substr;
                        substr++;
                    }
                    auto* node = id_char_symbols.find(
                        text.c_str(), text.size()
                    );
                    if (node && node->leaf) {
                        return {node->val, text};
                    } else {
                        return {sym_t::ID, text};
                    }
                }
            }
        }

        inline vector<Token> tokenize(const string& str) {
            vector<Token> tokens;
            const char* substr = str.c_str();
            while (substr < str.c_str()+str.size()) {
                while (char_type(*substr) == sym_t::WS) {substr++;}
                if (substr >= str.c_str()+str.size()) {break;}
                Token nt = next_token(substr);
                if (nt.type == sym_t::INVALID || nt.text.size() == 0) {
                    string prev = tokens.empty()? "<start>" : tokens.back().text;
                    cout << "Invalid token after " << prev << ": " << nt.text << '\n';
                    throw std::runtime_error("Invalid token");
                }
                tokens.push_back(nt);
                substr += nt.text.size();
            }
            return tokens;
        }
    }
}