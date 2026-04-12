#pragma once

#include "base.hpp"
#include "ds/trie_map.hpp"

namespace compiler {
    enum class sym_t {
        INVALID = 0,

        // TERMINALS
        // ignore
        WS, COMMENT,
        // seperators
        SEMI, COMMA,
        // values
        INT, FLOAT, ID, STRING,
        // brackets
        CURLY_LEFT, CURLY_RIGHT,
        SQUARE_LEFT, SQUARE_RIGHT,
        ROUND_LEFT, ROUND_RIGHT,
        // operators
        OPERATOR,
        // keywords
        IF, ELSE, FOR, WHILE, RETURN, IN,
        // type
        TYPE,

        // NONTERMINALS
        block, index, params,
        asn, type, expr,
        c_if, c_for, c_while,
        fdef, fcall
    };
    inline unordered_set<sym_t> terminals = {
        sym_t::SEMI, sym_t::COMMA,
        sym_t::INT, sym_t::FLOAT, sym_t::ID, sym_t::STRING,
        sym_t::CURLY_LEFT, sym_t::CURLY_RIGHT,
        sym_t::SQUARE_LEFT, sym_t::SQUARE_RIGHT,
        sym_t::ROUND_LEFT, sym_t::ROUND_RIGHT,
        sym_t::OPERATOR,
        sym_t::IF, sym_t::ELSE, sym_t::FOR, sym_t::WHILE, sym_t::RETURN, sym_t::IN,
        sym_t::TYPE
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
        else if (x == sym_t::block) {out << "block";}
        else if (x == sym_t::index) {out << "index";}
        else if (x == sym_t::params) {out << "params";}
        else if (x == sym_t::asn) {out << "asn";}
        else if (x == sym_t::type) {out << "type";}
        else if (x == sym_t::expr) {out << "expr";}
        else if (x == sym_t::c_if) {out << "c_if";}
        else if (x == sym_t::c_for) {out << "c_for";}
        else if (x == sym_t::c_while) {out << "c_while";}
        else if (x == sym_t::fdef) {out << "fdef";}
        else if (x == sym_t::fcall) {out << "fcall";}
        else {out << "UNKNOWN";}
        return out;
    }

    inline sym_t char_type(char c) {
        if (isspace(c)) {
            return sym_t::WS;
        } else if (c == '"' || c == '\'') {
            return sym_t::STRING;
        } else if (in_range(c, 'a', 'z') || in_range(c, 'A', 'Z') || in_range(c, '0', '9') || c == '_') {
            return sym_t::ID;
        } else {
            return sym_t::OPERATOR;
        }
    }

    struct Token {
        sym_t type;
        string text;
    };

    inline ds::trie_map<sym_t> op_char_symbols;
    inline ds::trie_map<sym_t> id_char_symbols;

    namespace symbols {
        inline void init() {
            vector<string> builtin_types = {
                "char", "int8", "int16", "int", "int32", "int64", "float64"
            };
            vector<string> operators = {
                // arith
                "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%=", "++", "--",
                // bit
                "&", "|", "^", "<<", ">>", "&=", "|=", "^=", "<<=", ">>=",
                // comp
                "==", "!=", "<", ">",  "<=", ">=",
                // bool
                "&&", "||",
                // other
                "=", "."
            };

            for (string& str: operators) {
                op_char_symbols.insert(str)->val = sym_t::OPERATOR;
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