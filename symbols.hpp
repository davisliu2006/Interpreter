#pragma once

#include "base.hpp"
#include "ds/trie_map.hpp"

namespace compiler {
    enum class sym_t {
        INVALID = 0,
        // ignore
        WS, COMMENT,
        // seperators
        SEMI, COMMA,
        // values
        INT, FLOAT, ID, STRING, CHAR,
        // brackets
        CURLY_LEFT, CURLY_RIGHT,
        SQUARE_LEFT, SQUARE_RIGHT,
        ROUND_LEFT, ROUND_RIGHT,
        // operators
        OPERATOR,
        // keywords
        IF, ELSE, FOR, WHILE, RETURN, IN,
        // type
        TYPE
    };

    inline std::ostream& operator <<(std::ostream& out, sym_t x) {
        if (x == sym_t::INVALID) {out << "INVALID";}
        else if (x == sym_t::WS) {out << "WS";}
        else if (x == sym_t::COMMENT) {out << "COMMENT";}
        else if (x == sym_t::SEMI) {out << "SEMI";}
        else if (x == sym_t::COMMA) {out << "COMMA";}
        else if (x == sym_t::INT) {out << "INT";}
        else if (x == sym_t::FLOAT) {out << "FLOAT";}
        else if (x == sym_t::ID) {out << "ID";}
        else if (x == sym_t::STRING) {out << "STRING";}
        else if (x == sym_t::CHAR) {out << "CHAR";}
        else if (x == sym_t::CURLY_LEFT) {out << "CURLY_LEFT";}
        else if (x == sym_t::CURLY_RIGHT) {out << "CURLY_RIGHT";}
        else if (x == sym_t::SQUARE_LEFT) {out << "SQUARE_LEFT";}
        else if (x == sym_t::SQUARE_RIGHT) {out << "SQUARE_RIGHT";}
        else if (x == sym_t::ROUND_LEFT) {out << "ROUND_LEFT";}
        else if (x == sym_t::ROUND_RIGHT) {out << "ROUND_RIGHT";}
        else if (x == sym_t::OPERATOR) {out << "OPERATOR";}
        else if (x == sym_t::IF) {out << "IF";}
        else if (x == sym_t::ELSE) {out << "ELSE";}
        else if (x == sym_t::FOR) {out << "FOR";}
        else if (x == sym_t::WHILE) {out << "WHILE";}
        else if (x == sym_t::RETURN) {out << "RETURN";}
        else if (x == sym_t::IN) {out << "IN";}
        else if (x == sym_t::TYPE) {out << "TYPE";}
        else {out << "UNKNOWN";}
        return out;
    }

    inline sym_t char_type(char c) {
        if (c == '\0') {
            return sym_t::INVALID;
        } else if (isspace(c)) {
            return sym_t::WS;
        } else if (c == '"' || c == '\'') {
            return sym_t::STRING;
        } else if (in_range(c, 'a', 'z') || in_range(c, 'A', 'Z') || in_range(c, '0', '9') || c == '_') {
            return sym_t::ID;
        } else {
            return sym_t::OPERATOR;
        }
    }
    inline bool is_opening_bracket(sym_t type) {
        return type == sym_t::ROUND_LEFT || type == sym_t::SQUARE_LEFT || type == sym_t::CURLY_LEFT;
    }
    inline bool is_closing_bracket(sym_t type) {
        return type == sym_t::ROUND_RIGHT || type == sym_t::SQUARE_RIGHT || type == sym_t::CURLY_RIGHT;
    }
    inline sym_t bracket_pair_type(sym_t type) {
        if (type == sym_t::ROUND_LEFT) {return sym_t::ROUND_RIGHT;}
        else if (type == sym_t::SQUARE_LEFT) {return sym_t::SQUARE_RIGHT;}
        else if (type == sym_t::CURLY_LEFT) {return sym_t::CURLY_RIGHT;}
        else if (type == sym_t::ROUND_RIGHT) {return sym_t::ROUND_LEFT;}
        else if (type == sym_t::SQUARE_RIGHT) {return sym_t::SQUARE_LEFT;}
        else if (type == sym_t::CURLY_RIGHT) {return sym_t::CURLY_LEFT;}
        else {return sym_t::INVALID;}
    }
    inline bool is_literal(sym_t type) {
        return type == sym_t::INT || type == sym_t::FLOAT || type == sym_t::STRING;
    }

    struct Token {
        sym_t type;
        string text;
    };

    inline ds::trie_map<sym_t> op_char_symbols;
    inline ds::trie_map<sym_t> id_char_symbols;
    inline ds::trie_map<int> operator_precedence;

    namespace symbols {
        inline void init() {
            vector<string> builtin_types = {
                "char", "int8", "int16", "int", "int32", "int64", "float64"
            };
            vector<vector<string>> operators = {
                {"."},
                {"++", "--"},
                {"*", "/", "%"},
                {"+", "-"},
                {"<<", ">>"},
                {"&"},
                {"^"},
                {"|"},
                {"^", "^=", "|", "|="},
                {"==", "!=", "<", ">", "<=", ">="},
                {"&&"},
                {"||"},
                {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}
            };

            for (int i = 0; i < operators.size(); i++) {
                for (const string& str: operators[i]) {
                    op_char_symbols.insert(str)->val = sym_t::OPERATOR;
                    operator_precedence.insert(str)->val = i;
                }
            }
            op_char_symbols.insert(";")->val = sym_t::SEMI;
            op_char_symbols.insert(",")->val = sym_t::COMMA;
            op_char_symbols.insert("{")->val = sym_t::CURLY_LEFT;
            op_char_symbols.insert("}")->val = sym_t::CURLY_RIGHT;
            op_char_symbols.insert("[")->val = sym_t::SQUARE_LEFT;
            op_char_symbols.insert("]")->val = sym_t::SQUARE_RIGHT;
            op_char_symbols.insert("(")->val = sym_t::ROUND_LEFT;
            op_char_symbols.insert(")")->val = sym_t::ROUND_RIGHT;

            for (string& str: builtin_types) {
                id_char_symbols.insert(str)->val = sym_t::TYPE;
            }
            id_char_symbols.insert("if")->val = sym_t::IF;
            id_char_symbols.insert("else")->val = sym_t::ELSE;
            id_char_symbols.insert("for")->val = sym_t::FOR;
            id_char_symbols.insert("while")->val = sym_t::WHILE;
            id_char_symbols.insert("return")->val = sym_t::RETURN;
            id_char_symbols.insert("in")->val = sym_t::IN;
        }
    }
}